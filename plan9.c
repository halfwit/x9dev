#define _PLAN9_SOURCE
#include <u.h>
#include <sys/types.h>
#include <libc.h>

#include <keyboard.h>
#include <draw.h>
#include "x9dev.h"

extern void ErrorF(char *, ...);
extern void FatalError(char *, ...);

x9devInfo x9di;

void
x9devInfoInit(void)
{
    char    buf[256];

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
    x9di.mouseFd = open(buf, O_RDWR | O_NONBLOCK);
    if(x9di.mouseFd < 0)
        FatalError("can't open mouse");

    snprint(buf, sizeof buf, "%s/cons", display->devdir);
    x9di.keybdFd = open(buf, O_RDONLY | O_NONBLOCK);
    if(x9di.keybdFd < 0)
        FatalError("can't open keyboard");

    snprint(buf, sizeof buf, "%s/consctl", display->devdir);
    x9di.consctlFd = open(buf, O_WRONLY);
    if(x9di.consctlFd < 0)
        FatalError("can't open consctl");
    if(write(x9di.consctlFd, "rawon", 5) != 5)
        FatalError("can't set rawon");
}

void
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
    flushimage(display, 1);
}

static void
x9devResize(void)
{
    if (getwindow(display, Refnone) < 0)
        FatalError("can't reattach to window");

    draw(screen, screen->r, display->white, nil, ZP);
    x9devRefreshScreen(0, 0, x9di.width, x9di.height);
}

int
x9devMouseRead(int *x, int *y, int *b)
{
    char    buf[1+4*12];
    int n;

    if ((n = read(x9di.mouseFd, buf, sizeof buf)) <= 0)
        return 0;

    if (n != 1 + 4 * 12)
        FatalError("Bad mouse event");

    if (buf[0] == 'r') {
        x9devResize();
        return 0;
    }
    *x = atoi(buf + 1 + 0 * 12) - screen->r.min.x;
    *y = atoi(buf + 1 + 1 * 12) - screen->r.min.y;
    *b = atoi(buf + 1 + 2 * 12);

    return 1;
}

wchar_t
x9devKeybdRead(void)
{
    static char s[3];
    static int  n = 0;
    wchar_t rune;

    if (read(x9di.keybdFd, s + n, 1) != 1)
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
