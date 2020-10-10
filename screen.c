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
x9devRefreshScreen(int x1, int y1, int x2, int y2)
{ 
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
    flushimage(_display, 1);
}


static void
x9devResize(void)
{
    if (getwindow(_display, Refnone) < 0)
        FatalError("can't reattach to window");

    draw(screen, screen->r, _display->white, nil, ZP);
    x9devRefreshScreen(0, 0, x9di.width, x9di.height);
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
x9devShadowUpdate(ScreenPtr pScreen, shadowBufPtr pBuf)
{
    BoxPtr pbox;

    pbox = REGION_EXTENTS(pScreen, &pBuf->pDamage);
    x9devRefreshScreen(pbox->x1, pbox->y1, pbox->x2, pbox->y2);
}


static void
x9devCursorLimits(DeviceIntPtr dptr, ScreenPtr spr, CursorPtr cpr, BoxPtr hot, BoxPtr topleft)
{
    *topleft = *hot;
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
