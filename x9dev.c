/*
 * Copyright (c) 2020 Michael Misch <michaelmisch1985@gmail.com> 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <errno.h>
#include "x9dev.h"

extern x9devInfo x9di;

/* 
 * 9p functions, used in draw and here 
 */
static int
wrsend(C9aux *a)
{
	uint32_t n;
	int w;

	for (n = 0; n < a->wroff; n += w) {
		if ((w = write(a->f, a->wrbuf+n, a->wroff-n)) <= 0) {
			if (errno == EINTR)
				continue;
			if (errno != EPIPE) /* remote end closed */
				perror("write");
			return -1;
		}
	}
	a->wroff = 0;

	return 0;
}

static uint8_t *
ctxbegin(C9ctx *ctx, uint32_t size)
{
    /* Should return a buffer to store 'size' bytes. Nil means no memory. */
	uint8_t *b;
	C9aux *a;

	a = ctx->aux;
	if (a->wroff + size > sizeof(a->wrbuf)) {
		if (wrsend(a) != 0 || a->wroff + size > sizeof(a->wrbuf))
			return NULL;
	}
	b = a->wrbuf + a->wroff;
	a->wroff += size;

	return b;
}

static int
ctxend(C9ctx *ctx)
{
    /*
	 * Marks the end of a message. Callback may decide if any accumulated
	 * messages should be sent to the server/client.
	 */
	(void)ctx;
	return 0;
}

static uint8_t *
ctxread(C9ctx *ctx, uint32_t size, int *err)
{
    /*
	 * Should return a pointer to the data (exactly 'size' bytes) read.
	 * Set 'err' to non-zero and return NULL in case of error.
	 * 'err' set to zero (no error) should be used to return from c9process
	 * early (timeout on read to do non-blocking operations, for example).
	 */
	uint32_t n;
	int r;
	C9aux *a;

	a = ctx->aux;
	*err = 0;
	for (n = 0; n < size; n += r) {
		if ((r = read(a->f, a->rdbuf+n, size-n)) <= 0) {
			if (errno == EINTR)
				continue;
			close(a->f);
			return NULL;
		}
	}

	return a->rdbuf;
}

__attribute__ ((format (printf, 1, 2)))
static void
ctxerror(const char *fmt, ...)
{

	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}


static void
x9r(C9ctx *ctx, C9r *r)
{
    /* Callback called every time a new R-message is received. */
	C9aux *a;
	C9tag tag;
	const char *path[2];
	char buf[64];

	a = ctx->aux;
	switch (r->type) {
	case Rread:
		break;
	case Ropen:
		break;
	case Rerror:
		break;
	default:
		break;
	}
}

void
x9devInfoInit(void)
{
    C9tag *cons;
    C9fid cfd;
    C9aux *c;

	c = calloc(1, sizeof(*c));
	c->ctx = calloc(1, sizeof(c->ctx));
	c->ctx->read = ctxread;
	c->ctx->begin = ctxbegin;
	c->ctx->end = ctxend;
	c->ctx->error = ctxerror;
	c->ctx->aux = c;
    c->ctx->r = x9r;

    /* We have 9p, we can init */
    //if(initdraw(c->ctx, "x9dev")< 0)
    if(initdraw(NULL, 0, "x9dev") < 0)
        FatalError("can't open display");

    x9di.depth = screen->depth;
    x9di.width = Dx(screen->r);
    x9di.width -= x9di.width&(screen->depth/8);
    x9di.dpi = 100;
    x9di.bpl = bytesperline(Rect(0, 0, x9di.width, x9di.height), x9di.depth);
    x9di.fb = malloc(x9di.bpl * x9di.height);
    x9di.ctx = c->ctx;

    sprintf(path, "%s/mouse", _display->devdir);
    c9walk(x9di.ctx, &x9di.mouse->tag, 1, x9di.mouse->f, &path);
    c9open(x9di.ctx, &x9di.mouse->tag, x9di.mouse->f, O_RDWR|O_NONBLOCK);

    sprintf(path, "%s/cons", _display->devdir);
    c9walk(x9di.ctx, &x9di.keybd->tag, 1, x9di.keybd->f, &path);
    c9open(x9di.ctx, &x9di.keybd->tag, x9di.keybd->f, O_RDONLY|O_NONBLOCK);

    sprintf(path, "%s/consctl", _display->devdir);
    c9walk(x9di.ctx, &cons, 1, cfd, &path);
    c9open(x9di.ctx, &cons, cfd, O_WRONLY);
    c9write(x9di.ctx, &cons, cfd, 0, "rawon", 5);
}

void
InitOutput(ScreenInfo *si, int argc, char *argv[])
{
    int i;

    si->imageByteOrder = IMAGE_BYTE_ORDER;
    si->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    si->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    si->bitmapBitOrder = BITMAP_BIT_ORDER;
    si->numPixmapFormats = NUMFORMATS;
    for (i = 0; i < NUMFORMATS; i++)
        si->formats[i] = formats[i];
    if (AddScreen(x9devScreenInit, argc, argv) < 0)
        FatalError("InitOutput: can't addscreen");
}

void
InitInput(int argc, char *argv[])
{
    Atom xiclass;
    
    x9devMouse = AddInputDevice(serverClient, x9devMouseProc, TRUE);
    x9devKeybd = AddInputDevice(serverClient, x9devKeybdProc, TRUE);
    xiclass = MakeAtom(XI_MOUSE, sizeof(XI_MOUSE) - 1, TRUE);
    AssignTypeAndName(x9devMouse, xiclass, "x9dev mouse");
    xiclass = MakeAtom(XI_KEYBOARD, sizeof(XI_KEYBOARD) - 1, TRUE);
    AssignTypeAndName(x9devKeybd, xiclass, "x9dev keyboard");

    mieqInit();
}

void
ddxUseMsg(void)
{
    ErrorF("\nx9srv Option Usage:\n");
    ErrorF("-D enable debug\n");
}

int
ddxProcessArgument(int argc, char **argv, int i)
{
    if (!strcmp(argv[i], "-D")){
        debug++;
        return 1;
    }

    return 0;
}

void
ddxGiveUp(enum ExitCode error)
{
}


void
AbortDDX(enum ExitCode error)
{
    ;
}

/* we may need to do some weird stuff here in the future */
#if defined(DDXBEFORERESET)
void
ddxBeforeReset(void)
{
}
#endif


void
DDXRingBell(int volume, int pitch, int duration)
{
}

void
OsVendorInit(void)
{

}

Bool
LegalModifier(unsigned int k, DeviceIntPtr pDev)
{
    return x9checkmod(k, pDev);
}

void
ProcessInputEvents(void)
{
    mieqProcessInputEvents();
}
