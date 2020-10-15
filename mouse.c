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

extern DeviceIntPtr x9devMouse;
int oldx, oldy, oldbut;

#define e    ev.u.u
#define ek    ev.u.keyButtonPointer
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

static int
x9devMouseRead(int *x, int *y, int *b)
{
    int n;
    char buf[1+4*12];

    if((n = read(x9di.mfd, buf, 1 + 4 * 12)) < = 0)
        return 0;

    if (n != 1 + 4 * 12)
        FatalError("Bad mouse event");

    if (buf == 'r') {
        x9devResize();
        return 0;
    }

    *x = atoi(buf + 1 + 0 * 12) - screen->r.min.x;
    *y = atoi(buf + 1 + 1 * 12) - screen->r.min.y;
    *b = atoi(buf + 1 + 2 * 12);

    return 1;
}


int  
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

int  
x9devMouseProc(DeviceIntPtr pDevice, int what)
{
    static unsigned char    map[] = {0, 1, 2, 3, 4, 5};
    DevicePtr pDev = (DevicePtr)pDevice;
    Atom btn_labels[3] = {0};
    Atom axes_labels[2] = {0};

    /* Init our globals */
    oldx = oldy = oldbut = 0;

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