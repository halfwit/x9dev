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
#include "glx_extinit.h"

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
    // Passed in arg[i], act on it
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
    return modmap[k] != 0;
}

void
ProcessInputEvents(void)
{
    mieqProcessInputEvents();
}