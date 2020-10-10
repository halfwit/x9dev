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

x9devInfo x9di;

extern DeviceIntPtr x9devMouse;
extern DeviceIntPtr x9devKeybd;
static int  oldx, oldy, oldbut;

static CARD8 modmap[MAP_LENGTH];

#define x9devSaveScreen (void *) NoopDDA
#define x9devConstrainCursor    (void *) NoopDDA
#define x9devDisplayCursor  (void *) NoopDDA
#define x9devRealizeCursor  (void *) NoopDDA
#define x9devUnrealizeCursor    (void *) NoopDDA
#define x9devRecolorCursor  (void *) NoopDDA
#define x9devSetCursorPosition  (void *) NoopDDA

static int x9read(C9ctx *, x9file *, uint32_t);

#define e    ev.u.u
#define ek    ev.u.keyButtonPointer
static void
x9devSendKeybdEvent(int k, int t)
{
    xEvent ev;

    memset(&ev, 0, sizeof(xEvent));
    e.type = t;
    e.detail = k + MIN_KEYCODE;
    ek.time = GetTimeInMillis();
    mieqEnqueue(x9devKeybd, (InternalEvent *)&ev);
}

static void
x9devSendMouseEvent(int x, int y, int b, int t)
{
    xEvent ev;

    memset(&ev, 0, sizeof(xEvent));
    e.type = t;
    e.detail = b;
    ek.rootX = x;
    ek.rootY = y;
    ek.time = GetTimeInMillis();
    mieqEnqueue(x9devMouse, (InternalEvent *)&ev);
}
#undef ek
#undef e

static void
x9devRefreshScreen(int x1, int y1, int x2, int y2)
{
    /*
    Rectangle r;
    uchar *p;
    int n;

    p = (uchar *)x9di.fb+(y1*x9di.width+x1)*(x9di.depth/8);
    r = rectaddpt(Rect(x1, y1, x2, y2), screen->r.min);
    if (Dx(r) == x9di.width) {
        if (loadimage(screen, r, p, x9di.bpl * Dy(r)) < 0)
            FatalError("can't load image");
        goto End;
    }
    n = Dx(r)*(x9di.depth/8);
    y2 += screen->r.min.y;
       while(r.min.y < y2) {
        r.max.y = r.min.y+1;
        if (loadimage(screen, r, p, n) < 0)
            FatalError("can't load image");
        p += x9di.bpl;
        r.min.y++;
    }

End:
    flushimage(display, 1);
    */
}

static void
x9devResize(void)
{
    /* 
    if (getwindow(display, Refnone) < 0)
        FatalError("can't reattach to window");

    draw(screen, screen->r, display->white, nil, ZP);
    x9devRefreshScreen(0, 0, x9di.width, x9di.height);
    */
}

static int
x9devMouseRead(int *x, int *y, int *b)
{
    int n;

    /* Magic numbers here are the size of a message from /dev/mouse and its offsets */
    if((n = x9read(x9di.ctx, x9di.mouse, 1 + 4 * 12)) <= 0)
        return 0;

    if (n != 1 + 4 * 12)
        FatalError("Bad mouse event");

    if (buf[0] == 'r') {
        x9devResize();
        return 0;
    }
    *x = atoi(x9di.mouse->rbuf + 1 + 0 * 12) - screen->r.min.x;
    *y = atoi(x9di.mouse->rbuf + 1 + 1 * 12) - screen->r.min.y;
    *b = atoi(x9di.mouse->rbuf + 1 + 2 * 12);

    return 1;
}

static wchar_t
x9devKeybdRead(void)
{
    static char s[3];
    static int  n = 0;
    wchar_t rune;

    if (x9read(x9di.ctx, x9di.keydb, 1) != 1)
        return 0;

    rune = s[0];
    if (n > 0 || (rune & 0x80) != 0x00) {
        if (mbtowc(&rune, s, n + 1) == -1) {
            /* incomplete rune; wait until next char */
            if (++n == 3)
                n = 0;
            return 0;
        }
        n = 0;
    }
    if (rune == Kdown)
        rune = 0x99;
    else if (rune & KF)
        rune = (rune&~KF) + 0x80;

    return rune;
}

static void
x9devInfoInit(void)
{
    /* 
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

static int  
x9devKeybdHandle(void)
{
    unsigned char   k, m;
    int c;

    c = x9devKeybdRead();
    if (c == 0 || c > sizeof(rune2keycode))
        return 0;

    k = rune2keycode[c].key;
    if (k == 0)
        return 0;

    m = rune2keycode[c].mod;
    if (m) 
        x9devSendKeybdEvent(m, KeyPress);

    x9devSendKeybdEvent(k, KeyPress);
    x9devSendKeybdEvent(k, KeyRelease);
    if (m) 
        x9devSendKeybdEvent(m, KeyRelease);

    return 1;
}


static int  
x9devMouseHandle(void)
{
    int x, y, b, but, t;

    if (!x9devMouseRead(&x, &y, &but))
        return 0;

    t = b = 0;
    if (x != oldx || y != oldy) {
        t = MotionNotify;
        oldx = x, oldy = y;
    }
    if (but != oldbut) {
        b = oldbut ^ but;
        t = ButtonPress;
        if (oldbut & b)
            t = ButtonRelease;
        if (b == 4)
            b = 3;
        if (but & (8 | 16)) {
            if (b & 8)
                b = 4;
            else
                b = 5;
            x9devSendMouseEvent(x, y, b, ButtonPress);
            x9devSendMouseEvent(x, y, b, ButtonRelease);
            return 1;
        }
    }
    x9devSendMouseEvent(x, y, b, t);
    oldbut = but;

    return 1;
}


static void
x9devWakeupHandler(ScreenPtr scr, int result)
{

    if (result <= 0)
        return;

    while (x9devMouseHandle())
        ;
    while (x9devKeybdHandle())
        ;
}


static void
x9devInitModmap(void)
{
    KeySym * ks;
    int i;

    for (i = 0; i < MAP_LENGTH; i++)
        modmap[i] = NoSymbol;

    for (i = MIN_KEYCODE, ks = map; i < (MIN_KEYCODE + NUM_KEYCODES); i++, ks += MAP_WIDTH)
        switch (*ks) {
        case XK_Shift_L:
        case XK_Shift_R:
            modmap[i] = ShiftMask;
            break;
        case XK_Control_L:
        case XK_Control_R:
            modmap[i] = ControlMask;
            break;
        case XK_Alt_L:
        case XK_Alt_R:
            modmap[i] = Mod1Mask;
            break;
        }
}

int  
x9devKeybdProc(DeviceIntPtr pDev, int what)
{

    switch (what) {
    case DEVICE_INIT:
        x9devInitModmap();
        if (!InitKeyboardDeviceStruct(pDev, NULL, 
            (BellProcPtr)NoopDDA, (KbdCtrlProcPtr)NoopDDA))
            FatalError("can't init keyboard");
	pDev->inited = TRUE;
        break;
    case DEVICE_ON:
        pDev->enabled = TRUE;
        break;
    case DEVICE_CLOSE:
	break;
    case DEVICE_OFF:
        pDev->enabled = FALSE;
        break;
    }
    return Success;
}

int  
x9devMouseProc(DeviceIntPtr pDevice, int what)
{
    static unsigned char    map[] = {0, 1, 2, 3, 4, 5};
    DevicePtr pDev = (DevicePtr)pDevice;
    Atom btn_labels[3] = {0};
    Atom axes_labels[2] = {0};

    switch (what) {
    case DEVICE_INIT:
        btn_labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
        btn_labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
        btn_labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);

        axes_labels[0] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_X);
        axes_labels[1] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_Y);
        InitPointerDeviceStruct(pDev, map, 3, btn_labels,
            (PtrCtrlProcPtr)NoopDDA, GetMotionHistorySize(), 2, axes_labels);
        break;

    case DEVICE_ON:
        pDev->on = TRUE;
        break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
        pDev->on = FALSE;
        break;
    }
    return Success;
}


static void
x9devCursorLimits(DeviceIntPtr dptr, ScreenPtr spr, CursorPtr cpr, BoxPtr hot, BoxPtr topleft)
{
    *topleft = *hot;
}

static void
x9devShadowUpdate(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    BoxPtr pbox;

    pbox = REGION_EXTENTS(pScreen, &pBuf->pDamage);
    x9devRefreshScreen(pbox->x1, pbox->y1, pbox->x2, pbox->y2);
}


/* callback dance... */
static CreateScreenResourcesProcPtr x9devCreateResourcesPtr;

static Bool
x9devCreateResources(ScreenPtr pScreen)
{
    Bool ret = 0;

    pScreen->CreateScreenResources = x9devCreateResourcesPtr;
    if (pScreen->CreateScreenResources)
        ret = pScreen->CreateScreenResources(pScreen);

    x9devCreateResourcesPtr = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = x9devCreateResourcesPtr;
    if (ret == FALSE)
        return FALSE;

    shadowRemove(pScreen, pScreen->GetScreenPixmap(pScreen));
    if (!shadowAdd(pScreen, pScreen->GetScreenPixmap(pScreen), x9devShadowUpdate, NULL, SHADOW_ROTATE_0, 0))
        return FALSE;

    return ret;
}

Bool
x9devScreenInit(ScreenPtr pScreen, int argc, char *argv[])
{
    int v, i;
    unsigned long   r, g, b;
    static int  first = 1;

    if (first) {
        x9devInfoInit();
        first = 0;
    }

    for (i = 0; i < NUMFORMATS; i++)
        fbSetVisualTypes(formats[i].depth, 0, 8);

    switch (x9di.depth) {
    case 16:
        r = 0xF800, g = 0x07E0, b = 0x001F;
        v = 1 << TrueColor;
        break;
    case 24:
    case 32:
        r = 0xFF0000, g = 0x00FF00, b = 0x0000FF;
        v = 1 << TrueColor;
        break;
    default:
        r = g = b = 0;
        v = 1 << PseudoColor;
    }
    if (!fbSetVisualTypesAndMasks(x9di.depth, v, 8, r, g, b))
        return FALSE;

    if (!fbScreenInit(pScreen, x9di.fb, x9di.width, x9di.height,
        x9di.dpi, x9di.dpi, x9di.width, x9di.depth))
        return FALSE;

    pScreen->mmWidth = x9di.width * 25.4 / x9di.dpi;
    pScreen->mmHeight = x9di.height * 25.4 / x9di.dpi;

    /* cursor */
    pScreen->ConstrainCursor = x9devConstrainCursor;
    pScreen->CursorLimits = x9devCursorLimits;
    pScreen->DisplayCursor = x9devDisplayCursor;
    pScreen->RealizeCursor = x9devRealizeCursor;
    pScreen->UnrealizeCursor = x9devUnrealizeCursor;
    pScreen->RecolorCursor = x9devRecolorCursor;
    pScreen->SetCursorPosition = x9devSetCursorPosition;

    pScreen->SaveScreen = x9devSaveScreen;
    pScreen->WakeupHandler = x9devWakeupHandler;

#ifdef RENDER
    if (!fbPictureInit(pScreen, 0, 0))
        return FALSE;
#endif

   if (!shadowSetup(pScreen))
        return FALSE;

    x9devCreateResourcesPtr = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = x9devCreateResources;

    if (!fbCreateDefColormap(pScreen))
        return FALSE;

    return TRUE;
}

Bool
x9checkmod(unsigned int k, DeviceIntPtr pDev)
{
    return modmap[k] != 0;
}

static int
x9read(C9ctx *ctx, x9file *file, uint32_t count)
{
    return c9read(ctx, &file->tag, 0, file->wroff, count);
}