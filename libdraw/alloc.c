
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "draw.h"

Image*
allocimage(Display *d, Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	i = _allocimage(nil, d, r, chan, repl, col, 0, 0);
	return i;
}

Image*
_allocimage(Image *ai, Display *d, Rectangle r, ulong chan, int repl, ulong col, int screenid, int refresh)
{
	uchar *a;
	char *err;
	Image *i;
	Rectangle clipr;
	int id;
	int depth;

	err = nil;
	i = nil;

	if(badrect(r)){
		return nil;
	}
	if(chan == 0){
		return nil;
	}

	depth = chantodepth(chan);
	if(depth == 0){
    Error:
		free(i);
		return nil;
	}

	a = bufimage(d, 1+4+4+1+4+1+4*4+4*4+4);
	if(a == nil)
		goto Error;
	d->imageid++;
	id = d->imageid;
	a[0] = 'b';
	BPLONG(a+1, id);
	BPLONG(a+5, screenid);
	a[9] = refresh;
	BPLONG(a+10, chan);
	a[14] = repl;
	BPLONG(a+15, r.min.x);
	BPLONG(a+19, r.min.y);
	BPLONG(a+23, r.max.x);
	BPLONG(a+27, r.max.y);
	if(repl)
		/* huge but not infinite, so various offsets will leave it huge, not overflow */
		clipr = Rect(-0x3FFFFFFF, -0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF);
	else
		clipr = r;
	BPLONG(a+31, clipr.min.x);
	BPLONG(a+35, clipr.min.y);
	BPLONG(a+39, clipr.max.x);
	BPLONG(a+43, clipr.max.y);
	BPLONG(a+47, col);

	if(ai != nil)
		i = ai;
	else{
		i = malloc(sizeof(Image));
		if(i == nil){
			a = bufimage(d, 1+4);
			if(a != nil){
				a[0] = 'f';
				BPLONG(a+1, id);
				flushimage(d, 0);
			}
			goto Error;
		}
	}
	i->display = d;
	i->id = id;
	i->depth = depth;
	i->chan = chan;
	i->r = r;
	i->clipr = clipr;
	i->repl = repl;
	i->screen = nil;
	i->next = nil;
	return i;
}

Image*
namedimage(Display *d, char *name)
{
	uchar *a;
	char *err, buf[12*12+1];
	Image *i;
	int id, n;
	ulong chan;

	err = nil;
	i = nil;

	n = strlen(name);
	if(n >= 256){
    Error:
		free(i);
		return nil;
	}
	/* flush pending data so we don't get error allocating the image */
	flushimage(d, 0);
	a = bufimage(d, 1+4+1+n);
	if(a == nil)
		goto Error;
	d->imageid++;
	id = d->imageid;
	a[0] = 'n';
	BPLONG(a+1, id);
	a[5] = n;
	memmove(a+6, name, n);
	if(flushimage(d, 0) < 0)
		goto Error;

	lseek(d->ctlfd, 0, SEEK_SET);
	if(read(d->ctlfd, buf, sizeof buf) < 12*12)
		goto Error;
	buf[12*12] = '\0';

	i = malloc(sizeof(Image));
	if(i == nil){
	Error1:
		a = bufimage(d, 1+4);
		if(a != nil){
			a[0] = 'f';
			BPLONG(a+1, id);
			flushimage(d, 0);
		}
		goto Error;
	}
	i->display = d;
	i->id = id;
	if((chan=strtochan(buf+2*12))==0){
		goto Error1;
	}
	i->chan = chan;
	i->depth = chantodepth(chan);
	i->repl = atoi(buf+3*12);
	i->r.min.x = atoi(buf+4*12);
	i->r.min.y = atoi(buf+5*12);
	i->r.max.x = atoi(buf+6*12);
	i->r.max.y = atoi(buf+7*12);
	i->clipr.min.x = atoi(buf+8*12);
	i->clipr.min.y = atoi(buf+9*12);
	i->clipr.max.x = atoi(buf+10*12);
	i->clipr.max.y = atoi(buf+11*12);
	i->screen = nil;
	i->next = nil;
	return i;
}

int
nameimage(Image *i, char *name, int in)
{
	uchar *a;
	int n;

	n = strlen(name);
	a = bufimage(i->display, 1+4+1+1+n);
	if(a == nil)
		return 0;
	a[0] = 'N';
	BPLONG(a+1, i->id);
	a[5] = in;
	a[6] = n;
	memmove(a+7, name, n);
	if(flushimage(i->display, 0) < 0)
		return 0;
	return 1;
}

int
_freeimage1(Image *i)
{
	uchar *a;
	Display *d;
	Image *w;

	if(i == nil || i->display == nil)
		return 0;
	d = i->display;
	if(i->screen != nil){
		w = d->windows;
		if(w == i)
			d->windows = i->next;
		else
			while(w != nil){
				if(w->next == i){
					w->next = i->next;
					break;
				}
				w = w->next;
			}
	}
	a = bufimage(d, 1+4);
	if(a == nil)
		return -1;
	a[0] = 'f';
	BPLONG(a+1, i->id);
	return 0;
}

int
freeimage(Image *i)
{
	int ret;

	ret = _freeimage1(i);
	free(i);
	return ret;
}
