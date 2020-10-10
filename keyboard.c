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
#define KF      0xF000
#define Kdown   0x80

extern x9devInfo x9di;
extern DeviceIntPtr x9devKeybd;
static CARD8 modmap[MAP_LENGTH];

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

#undef ek
#undef e

static wchar_t
x9devKeybdRead(void)
{
    static char s[3];
    static int  n = 0;
    wchar_t rune;

    if (x9read(x9di.ctx, x9di.keybd, 1) != 1)
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

Bool
x9checkmod(unsigned int k, DeviceIntPtr pDev)
{
    return modmap[k] != 0;
}
