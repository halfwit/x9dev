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
#define PSZ 8
#include "fb.h" /* fbSetVisualTypes */
#include "colormapst.h"
#include "dix.h"
#include "exevents.h"
#include "extinit.h"
#include "gcstruct.h"
#include "glx_extinit.h"
#include "input.h"
#include "micmap.h"
#include "mipointer.h"
#include "miline.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "shadow.h"
#include "xkbsrv.h"
#include "xserver-properties.h"
#include "mi.h" /* miEnqueue mieqProcessInputEvents */

#include "c9/c9.h"
#include "libdraw/draw.h"

#define Msize 8192
#define NUMFORMATS (sizeof(formats)/sizeof((formats)[0]))

/* NOOPs for now */
#define x9devSaveScreen (void *) NoopDDA
#define x9devConstrainCursor    (void *) NoopDDA
#define x9devDisplayCursor  (void *) NoopDDA
#define x9devRealizeCursor  (void *) NoopDDA
#define x9devUnrealizeCursor    (void *) NoopDDA
#define x9devRecolorCursor  (void *) NoopDDA
#define x9devSetCursorPosition  (void *) NoopDDA


typedef struct x9file x9file;
struct x9file {
    C9tag    tag;
	uint8_t  rdbuf[Msize];
	uint8_t  wrbuf[Msize];
	uint32_t wroff;
};

typedef struct x9devInfo x9devInfo;
struct x9devInfo
{
    char    *fb;
    int     depth;
    int     width;
    int     height;
    int     dpi;
    int     bpl;
    C9ctx   *ctx;
    /* Our three files we care about */
    x9file  *mouse;
    x9file  *keybd;
    x9file  *draw;
};

int debug = 0;

static PixmapFormatRec formats[] = {
    { 1,    1,  BITMAP_SCANLINE_PAD },
    { 8,    8,  BITMAP_SCANLINE_PAD },
    { 16,   16,     BITMAP_SCANLINE_PAD },
    { 24,   24,     BITMAP_SCANLINE_PAD },
    { 32,   32,     BITMAP_SCANLINE_PAD }
};

DeviceIntPtr x9devMouse;
DeviceIntPtr x9devKeybd;
x9devInfo x9di;
Screen *screen;

Bool x9checkmod(unsigned int, DeviceIntPtr);
Bool x9devScreenInit(ScreenPtr, int, char **);
int x9devKeybdProc(DeviceIntPtr, int);
int x9devMouseProc(DeviceIntPtr, int);

#endif
