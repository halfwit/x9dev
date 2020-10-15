#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "draw.h"

Display	*_display;
_Font	*font;
Image	*screen;

enum{
	DISP_BUFSIZE = 8000, /* Play with this some, see if changes much */
};

static char deffontname[] = "*default*";
Screen	*_screen;

int	debuglockdisplay = 0;

static void _closedisplay(Display*, int);

/* note handler */
static void
drawshutdown(void)
{
	Display *d;

	d = _display;
	if(d != nil){
		_display = nil;
		_closedisplay(d, 1);
	}
}

int
geninitdraw(char *devdir, void(*error)(Display*, char*), char *label, char *windir, int ref)
{
	int fd;
	char *fontname;
	char buf[128];
	Subfont *df;

	_display = initdisplay(devdir, windir, error);
	if(_display == nil)
		return -1;

	/*
	 * Set up default font
	 */
	df = getdefont(_display);
	_display->defaultsubfont = df;
	if(df == nil){
    Error:
		closedisplay(_display);
		_display = nil;
		return -1;
	}

	fd = open("/env/font", O_RDONLY);
	read(fd, buf, sizeof(buf));
	fontname = buf;

	/*
	 * Build fonts with caches==depth of screen, for speed.
	 * If conversion were faster, we'd use 0 and save memory.
	 */
	if(strcmp(fontname, "") != 0){
		snprintf(buf, sizeof buf, "%d %d\n0 %d\t%s\n", df->height, df->ascent,
			df->n-1, deffontname);
//BUG: Need something better for this	installsubfont("*default*", df);
		font = buildfont(_display, buf, deffontname);
		if(font == nil)
			goto Error;
	}else{
		font = openfont(_display, fontname);	/* BUG: grey fonts */
		if(font == nil)
			goto Error;
	}
	_display->defaultfont = font;

	/*
	 * Write label; ignore errors (we might not be running under rio)
	 */
	if(label != nil){
		snprintf(buf, sizeof buf, "%s/label", _display->windir);
		fd = open(buf, O_RDONLY);
		if(fd >= 0){
			read(fd, _display->oldlabel, (sizeof _display->oldlabel)-1);
			close(fd);
			fd = open(buf, O_CREAT|O_WRONLY, 0666);
			if(fd >= 0){
				write(fd, label, strlen(label));
				close(fd);
			}
		}
	}

	snprintf(buf, sizeof buf, "%s/winname", _display->windir);
	if(gengetwindow(_display, buf, &screen, &_screen, ref) < 0)
		goto Error;

	atexit(drawshutdown);

	return 1;
}

int
initdraw(void(*error)(Display*, char*), char *label)
{
	static char dev[] = "/dev";

	return geninitdraw(dev, error, label, dev, Refnone);
}

/*
 * Attach, or possibly reattach, to window.
 * If reattaching, maintain value of screen pointer.
 */
int
gengetwindow(Display *d, char *winname, Image **winp, Screen **scrp, int ref)
{
	int n, fd;
	char buf[64+1], obuf[64+1];
	Image *image;
	Rectangle r;

	obuf[0] = 0;
retry:
	fd = open(winname, O_RDONLY);
	if(fd<0 || (n=read(fd, buf, sizeof buf-1))<=0){
		if((image=d->image) == nil){
			*winp = nil;
			d->screenimage = nil;
			return -1;
		}
		strcpy(buf, "noborder");
	}else{
		close(fd);
		buf[n] = '\0';
		image = namedimage(d, buf);
		if(image == nil){
			/*
			 * theres a race where the winname can change after
			 * we read it, so keep trying as long as the name
			 * keeps changing.
			 */
			if(strcmp(buf, obuf) != 0){
				strcpy(obuf, buf);
				goto retry;
			}
		}
		if(*winp != nil){
			_freeimage1(*winp);
			freeimage((*scrp)->image);
			freescreen(*scrp);
			*scrp = nil;
		}
		if(image == nil){
			*winp = nil;
			d->screenimage = nil;
			return -1;
		}
	}

	d->screenimage = image;
	*scrp = allocscreen(image, d->white, 0);
	if(*scrp == nil){
		*winp = nil;
		d->screenimage = nil;
		freeimage(image);
		return -1;
	}

	r = image->r;
	if(strncmp(buf, "noborder", 8) != 0)
		r = insetrect(image->r, Borderwidth);
	*winp = _allocwindow(*winp, *scrp, r, ref, DWhite);
	if(*winp == nil){
		freescreen(*scrp);
		*scrp = nil;
		d->screenimage = nil;
		freeimage(image);
		return -1;
	}
	d->screenimage = *winp;
	return 1;
}

int
getwindow(Display *d, int ref)
{
	char winname[128];

	snprintf(winname, sizeof winname, "%s/winname", d->windir);
	return gengetwindow(d, winname, &screen, &_screen, ref);
}

#define	NINFO	12*12

Display*
initdisplay(char *dev, char *win, void(*error)(Display*, char*))
{
	char buf[128], info[NINFO+1], *t, isnew;
	int n, datafd, ctlfd, reffd;
	Display *disp;
	Image *image;

	if(dev == nil)
		dev = "/dev";
	if(win == nil)
		win = "/dev";
	if(strlen(dev)>sizeof buf-25 || strlen(win)>sizeof buf-25){
		return nil;
	}
	t = strdup(win);
	if(t == nil)
		return nil;

	sprintf(buf, "%s/draw/new", dev);
	ctlfd = open(buf, O_RDWR);
	if(ctlfd < 0){
    Error1:
		free(t);

		return nil;
	}
	if((n=read(ctlfd, info, sizeof info)) < 12){
    Error2:
		close(ctlfd);
		goto Error1;
	}
	if(n==NINFO+1)
		n = NINFO;
	info[n] = '\0';
	isnew = 0;
	if(n < NINFO)	/* this will do for now, we need something better here */
		isnew = 1;
	sprintf(buf, "%s/draw/%d/data", dev, atoi(info+0*12));
	datafd = open(buf, O_RDWR);
	if(datafd < 0)
		goto Error2;
	sprintf(buf, "%s/draw/%d/refresh", dev, atoi(info+0*12));
	reffd = open(buf, O_RDWR);
	if(reffd < 0){
    Error3:
		close(datafd);
		goto Error2;
	}
	disp = calloc(sizeof(Display), 1);
	if(disp == nil){
    Error4:
		close(reffd);
		goto Error3;
	}
	image = nil;
	if(0){
    Error5:
		free(image);
		free(disp);
		goto Error4;
	}
	if(n >= NINFO){
		image = calloc(sizeof(Image), 1);
		if(image == nil)
			goto Error5;
		image->display = disp;
		image->id = 0;
		image->chan = strtochan(info+2*12);
		image->depth = chantodepth(image->chan);
		image->repl = atoi(info+3*12);
		image->r.min.x = atoi(info+4*12);
		image->r.min.y = atoi(info+5*12);
		image->r.max.x = atoi(info+6*12);
		image->r.max.y = atoi(info+7*12);
		image->clipr.min.x = atoi(info+8*12);
		image->clipr.min.y = atoi(info+9*12);
		image->clipr.max.x = atoi(info+10*12);
		image->clipr.max.y = atoi(info+11*12);
	}

	disp->_isnewdisplay = isnew;
	disp->bufsize = DISP_BUFSIZE;
	disp->buf = malloc(disp->bufsize+5);	/* +5 for flush message */
	if(disp->buf == nil)
		goto Error5;

	disp->image = image;
	disp->dirno = atoi(info+0*12);
	disp->fd = datafd;
	disp->ctlfd = ctlfd;
	disp->reffd = reffd;
	disp->bufp = disp->buf;
	disp->error = error;
	disp->windir = t;
	disp->devdir = strdup(dev);
	pthread_mutex_lock(&disp->qlock);
	disp->white = allocimage(disp, Rect(0, 0, 1, 1), GREY1, 1, DWhite);
	disp->black = allocimage(disp, Rect(0, 0, 1, 1), GREY1, 1, DBlack);
	if(disp->white == nil || disp->black == nil){
		free(disp->devdir);
		free(disp->white);
		free(disp->black);
		goto Error5;
	}
	disp->opaque = disp->white;
	disp->transparent = disp->black;

	/* We still need this */
	//dir = dirfstat(ctlfd);
	//if(dir!=nil && dir->type=='i'){
	//	disp->local = 1;
	//	disp->dataqid = dir->qid.path;
	//}
	//if(dir!=nil && dir->qid.vers==1)	/* other way to tell */
	//	disp->_isnewdisplay = 1;
	//free(dir);

	return disp;
}

/*
 * Call with d unlocked.
 * Note that disp->defaultfont and defaultsubfont are not freed here.
 */
void
closedisplay(Display *disp)
{
	_closedisplay(disp, 0);
}

static void
_closedisplay(Display *disp, int isshutdown)
{
	int fd;
	char buf[128];

	if(disp == nil)
		return;
	if(disp == _display)
		_display = nil;
	if(disp->oldlabel[0]){
		snprintf(buf, sizeof buf, "%s/label", disp->windir);
		fd = open(buf, O_WRONLY);
		if(fd >= 0){
			write(fd, disp->oldlabel, strlen(disp->oldlabel));
			close(fd);
		}
	}

	/*
	 * if we're shutting down, don't free all the resources.
	 * if other procs are getting shot down by notes too,
	 * one might get shot down while holding the malloc lock.
	 * just let the kernel clean things up when we exit.
	 */
	if(isshutdown)
		return;

	free(disp->devdir);
	free(disp->windir);
	freeimage(disp->white);
	freeimage(disp->black);
	close(disp->fd);
	close(disp->ctlfd);
	/* should cause refresh slave to shut down */
	close(disp->reffd);
	pthread_mutex_unlock(&disp->qlock);
	free(disp);
}

void
lockdisplay(Display *disp)
{
	if(debuglockdisplay){
		/* avoid busy looping; it's rare we collide anyway */
		while(!pthread_mutex_trylock(&disp->qlock))
			sleep(1000);
	}else
		pthread_mutex_lock(&disp->qlock);
}

void
unlockdisplay(Display *disp)
{
	pthread_mutex_unlock(&disp->qlock);
}

void
drawerror(Display *d, char *s)
{
	char err[128]; /* ERRMAX */

	if(d != nil && d->error != nil)
		(*d->error)(d, s);
	else{
		fprintf(stderr, "draw: %s: %s\n", s, err);
		exit(1);
	}
}

static
int
doflush(Display *d)
{
	int n;

	n = d->bufp-d->buf;
	if(n <= 0)
		return 1;

	if(write(d->fd, d->buf, n) != n){
		d->bufp = d->buf;	/* might as well; chance of continuing */
		return -1;
	}
	d->bufp = d->buf;
	return 1;
}

int
flushimage(Display *d, int visible)
{
	if(d == nil)
		return 0;
	if(visible){
		*d->bufp++ = 'v';	/* five bytes always reserved for this */
		if(d->_isnewdisplay){
			BPLONG(d->bufp, d->screenimage->id);
			d->bufp += 4;
		}
	}
	return doflush(d);
}

uchar*
bufimage(Display *d, int n)
{
	uchar *p;

	if(n<0 || n>d->bufsize){
		return nil;
	}
	if(d->bufp+n > d->buf+d->bufsize)
		if(doflush(d) < 0)
			return nil;
	p = d->bufp;
	d->bufp += n;
	return p;
}

