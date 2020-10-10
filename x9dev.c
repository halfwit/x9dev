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

#include "x9dev.h"

static void
x9devInfoInit(void)
{
    if(initdraw(NULL, 0, "x9dev") < 0)
        FatalError("can't open display");

    x9di.depth = screen->depth;
    x9di.width = Dx(screen->r);
    x9di.width -= x9di.width&(screen->depth/8);
    x9di.dpi = 100;
    x9di.bpl = bytesperline(Rect(0, 0, x9di.width, x9di.height), x9di.depth);
    x9di.fb = malloc(x9di.bpl * x9di.height);
    if (x9di.fb == nil)
        FatalError("couldn't allocate framebuffer");

/*
    snprint(buf, sizeof buf, "%s/mouse", display->devdir);
    x9di.mouseFd = c9open(buf, O_RDWR | O_NONBLOCK);
    if(x9di.mouseFd < 0)
        FatalError("can't open mouse");

    snprint(buf, sizeof buf, "%s/cons", display->devdir);
    x9di.keybdFd = c9open(buf, O_RDONLY | O_NONBLOCK);
    if(x9di.keybdFd < 0)
        FatalError("can't open keyboard");

    snprint(buf, sizeof buf, "%s/consctl", display->devdir);
    x9di.consctlFd = c9open(buf, O_WRONLY);
    if(x9di.consctlFd < 0)
        FatalError("can't open consctl");
    if(c9write(x9di.consctlFd, "rawon", 5) != 5)
        FatalError("can't set rawon");
*/
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