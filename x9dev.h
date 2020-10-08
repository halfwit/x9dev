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

#ifndef _X9DEV_H_
#define _X9DEV_H_

#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include <X11/Xpoll.h>
#define XK_TECHNICA
#define XK_PUBLISHING
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include "servermd.h"
#define PSZ 8
#include "fb.h"
#include "colormapst.h"
#include "gcstruct.h"
#include "mipointer.h"
#include "micmap.h"
#include "dix.h"
#include "miline.h"
#include "shadow.h"
#include "xkbsrv.h"
#include "xserver-properties.h"
#include "exevents.h"
#include "extinit.h"
#include "keymap.h"
#include "mi.h"
#include "glx_extinit.h"
#include "input.h"
#include "scrnintstr.h"
typedef struct x9devInfo x9devInfo;

struct x9devInfo
{
    char    *fb;
    int     depth;
    int     width;
    int     height;
    int     dpi;
    int     bpl;
    int     mouseFd;
    int     keybdFd;
    int     consctlFd;
};

#define KF      0xF000
#define Kdown   0x80

DeviceIntPtr x9devMouse;
DeviceIntPtr x9devKeybd;

Bool x9checkmod(unsigned int, DeviceIntPtr);
Bool x9devScreenInit(ScreenPtr, int, char **);
int x9devKeybdProc(DeviceIntPtr, int);
int x9devMouseProc(DeviceIntPtr, int);

#endif
