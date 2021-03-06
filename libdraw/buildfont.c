#include <stdlib.h>
#include <string.h>
#include "draw.h"

static char*
skip(char *s)
{
	while(*s==' ' || *s=='\n' || *s=='\t')
		s++;
	return s;
}

_Font*
buildfont(Display *d, char *buf, char *name)
{
	_Font *fnt;
	Cachefont *c, **sub;
	char *s, *t;
	ulong min, max;
	int offset;
	char badform[] = "bad font format: number expected (char position %d)";

	s = buf;
	fnt = malloc(sizeof(_Font));
	if(fnt == nil)
		return nil;
	memset(fnt, 0, sizeof(_Font));
	fnt->display = d;
	fnt->name = strdup(name);
	fnt->ncache = NFCACHE+NFLOOK;
	fnt->nsubf = NFSUBF;
	fnt->cache = malloc(fnt->ncache * sizeof(fnt->cache[0]));
	fnt->subf = malloc(fnt->nsubf * sizeof(fnt->subf[0]));
	if(fnt->name==nil || fnt->cache==nil || fnt->subf==nil){
    Err2:
		free(fnt->name);
		free(fnt->cache);
		free(fnt->subf);
		free(fnt->sub);
		free(fnt);
		return nil;
	}
	fnt->height = strtol(s, &s, 0);
	s = skip(s);
	fnt->ascent = strtol(s, &s, 0);
	s = skip(s);
	if(fnt->height<=0 || fnt->ascent<=0){
		goto Err2;
	}
	fnt->width = 0;
	fnt->nsub = 0;
	fnt->sub = nil;

	memset(fnt->subf, 0, fnt->nsubf * sizeof(fnt->subf[0]));
	memset(fnt->cache, 0, fnt->ncache*sizeof(fnt->cache[0]));
	fnt->age = 1;
	do{
		/* must be looking at a number now */
		if(*s<'0' || '9'<*s){
			goto Err3;
		}
		min = strtol(s, &s, 0);
		s = skip(s);
		/* must be looking at a number now */
		if(*s<'0' || '9'<*s){
			goto Err3;
		}
		max = strtol(s, &s, 0);
		s = skip(s);
		if(*s==0 || min>Runemax || max>Runemax || min>max){
    Err3:
			freefont(fnt);
			return 0;
		}
		t = s;
		offset = strtol(s, &t, 0);
		if(t>s && (*t==' ' || *t=='\t' || *t=='\n'))
			s = skip(t);
		else
			offset = 0;
		sub = realloc(fnt->sub, (fnt->nsub+1)*sizeof(Cachefont*));
		if(sub == nil)
			goto Err3;
		fnt->sub = sub;
		c = malloc(sizeof(Cachefont));
		if(c == nil)
			goto Err3;
		c->min = min;
		c->max = max;
		c->offset = offset;
		t = s;
		while(*s && *s!=' ' && *s!='\n' && *s!='\t')
			s++;
		*s++ = 0;
		c->subfontname = nil;
		c->name = strdup(t);
		if(c->name == nil){
			free(c);
			goto Err3;
		}
		sub[fnt->nsub++] = c;
		s = skip(s);
	}while(*s);
	return fnt;
}

void
freefont(_Font *f)
{
	int i;
	Cachefont *c;
	Subfont *s;

	if(f == nil)
		return;

	for(i=0; i<f->nsub; i++){
		c = f->sub[i];
		free(c->subfontname);
		free(c->name);
		free(c);
	}
	for(i=0; i<f->nsubf; i++){
		s = f->subf[i].f;
		if(s != nil){
			if(f->display == nil || s != f->display->defaultsubfont)
				freesubfont(s);
		}
	}
	freeimage(f->cacheimage);
	free(f->name);
	free(f->cache);
	free(f->subf);
	free(f->sub);
	free(f);
}
