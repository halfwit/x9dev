#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "c9/c9.h"

enum {
	Msize = 8192,

	Rootfid = 0,
	Indexfid,
	Chatfid,

	Error = 1<<0,
	Joined = 1<<1,
	Disconnected = 1<<2,
};

typedef struct C9aux C9aux;
typedef struct REntry REntry;

struct C9aux {
	C9ctx;
	int f;
	int flags;
	uint8_t rdbuf[Msize];
	uint8_t wrbuf[Msize];
	uint32_t wroff;
};

static const char *nick;
static int printjoin;
static uint64_t chatoff, skipuntil;

static uint8_t *
ctxread(C9ctx *ctx, uint32_t size, int *err)
{
	uint32_t n;
	int r;
	C9aux *a;

	a = ctx->aux;
	*err = 0;
	for (n = 0; n < size; n += r) {
		if ((r = read(a->f, a->rdbuf+n, size-n)) <= 0) {
			if (errno == EINTR)
				continue;
			a->flags |= Disconnected;
			close(a->f);
			return NULL;
		}
	}

	return a->rdbuf;
}

static int
wrsend(C9aux *a)
{
	uint32_t n;
	int w;

	for (n = 0; n < a->wroff; n += w) {
		if ((w = write(a->f, a->wrbuf+n, a->wroff-n)) <= 0) {
			if (errno == EINTR)
				continue;
			if (errno != EPIPE) /* remote end closed */
				perror("write");
			return -1;
		}
	}
	a->wroff = 0;

	return 0;
}

static uint8_t *
ctxbegin(C9ctx *ctx, uint32_t size)
{
	uint8_t *b;
	C9aux *a;

	a = ctx->aux;
	if (a->wroff + size > sizeof(a->wrbuf)) {
		if (wrsend(a) != 0 || a->wroff + size > sizeof(a->wrbuf))
			return NULL;
	}
	b = a->wrbuf + a->wroff;
	a->wroff += size;

	return b;
}

static int
ctxend(C9ctx *ctx)
{
	(void)ctx;
	return 0;
}

static int
dial(char *s)
{
	struct addrinfo *r, *a, hint = {.ai_flags = AI_ADDRCONFIG, .ai_family = AF_UNSPEC, 0};
	char host[64], *port;
	int e, f;

	if (strncmp(s, "udp!", 4) == 0) {
		hint.ai_socktype = SOCK_DGRAM;
		hint.ai_protocol = IPPROTO_UDP;
	} else if (strncmp(s, "tcp!", 4) == 0) {
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = IPPROTO_TCP;
	} else {
		fprintf(stderr, "invalid dial string: %s\n", s);
		return -1;
	}
	if ((port = strchr(s+4, '!')) == NULL) {
		fprintf(stderr, "invalid dial string: %s\n", s);
		return -1;
	}
	if (snprintf(host, sizeof(host), "%.*s", (int)(port-s-4), s+4) >= (int)sizeof(host)) {
		fprintf(stderr, "host name too large: %s\n", s);
		return -1;
	}
	port++;
	if ((e = getaddrinfo(host, port, &hint, &r)) != 0)
		return -1;
	f = -1;
	for (a = r; a != NULL; a = a->ai_next) {
		if ((f = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) < 0)
			continue;
		if (connect(f, a->ai_addr, a->ai_addrlen) != 0) {
			close(f);
			f = -1;
			continue;
		}
	}
	freeaddrinfo(r);

	return f;
}

static void
output(uint8_t *d, int sz)
{
	int i, j;

	for (i = j = 0; i < sz; i++) {
		d[j] = d[i];
		if (d[j] > 31 || d[j] == '\t' || d[j] == '\n')
			j++;
	}
	write(1, d, j);
}

static void
ctxchatR(C9ctx *ctx, C9r *r)
{
	C9aux *a;
	C9tag tag;
	const char *path[2];
	char buf[64];

	a = ctx->aux;
	switch (r->type) {
	case Rversion:
		c9attach(ctx, &tag, Rootfid, C9nofid, "none", NULL);
		path[0] = "chat";
		path[1] = NULL;
		c9walk(ctx, &tag, Rootfid, Chatfid, path);
		c9open(ctx, &tag, Chatfid, C9rdwr);
		break;

	case Rread:
		if (chatoff >= skipuntil)
			output(r->read.data, r->read.size);
		chatoff += r->read.size;
		/* fallthrough */
	case Ropen:
		if ((a->flags & Joined) == 0 && printjoin) {
			c9write(ctx, &tag, Chatfid, 0, buf, snprintf(buf, sizeof(buf), "JOIN %s to chat\n", nick));
			a->flags |= Joined;
		}
		c9read(ctx, &tag, Chatfid, chatoff, chatoff < skipuntil ? skipuntil-chatoff : Msize);
		break;

	case Rerror:
		fprintf(stderr, "chat error: %s\n", r->error);
		a->flags |= Error;
		break;

	default:
		break;
	}
}

static void
ctxregistryR(C9ctx *ctx, C9r *r)
{
	char *s, *b;
	C9aux *a;
	C9tag tag;
	const char *path[2];

	a = ctx->aux;
	switch (r->type) {
	case Rversion:
		c9attach(ctx, &tag, Rootfid, C9nofid, "none", NULL);
		path[0] = "index";
		path[1] = NULL;
		c9walk(ctx, &tag, Rootfid, Indexfid, path);
		c9open(ctx, &tag, Indexfid, C9read);
		break;

	case Ropen:
		c9read(ctx, &tag, Indexfid, 0, Msize);
		break;

	case Rread:
		r->read.data[r->read.size] = 0;
		for (s = (char*)r->read.data;;) {
			if ((s = strstr(s, "gridchat")) == NULL)
				break;
			for (b = s; b != (char*)r->read.data && *b != '\n'; b--);
			if (*b == '\n')
				b++;
			if ((s = strchr(s, '\n')) == NULL)
				s = (char*)&r->read.data[r->read.size];
			*s = 0;
			if (strstr(b, "tlssrv") == NULL && (s = strchr(b, ' ')) != NULL) {
				*s = 0;
				close(a->f);
				if ((a->f = dial(b)) < 0)
					exit(1);
				a->flags = 0;
				a->r = ctxchatR;
				a->wroff = 0;
				c9version(ctx, &tag, Msize);
				if (wrsend(a) != 0)
					exit(1);
				return;
			}
		}
		fprintf(stderr, "chat hasn't been found in the registry\n");
		exit(1);
		break;

	case Rerror:
		fprintf(stderr, "registry error: %s\n", r->error);
		a->flags |= Error;
		break;

	default:
		break;
	}
}

__attribute__ ((format (printf, 1, 2)))
static void
ctxerror(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

static C9aux *
srv(char *s)
{
	C9aux *c;
	int f;

	if ((f = dial(s)) < 0)
		return NULL;
	c = calloc(1, sizeof(*c));
	c->f = f;
	c->read = ctxread;
	c->begin = ctxbegin;
	c->end = ctxend;
	c->error = ctxerror;
	c->aux = c;

	return c;
}

static C9aux *
registry(void)
{
	C9aux *a;
	C9tag tag;
	int i;

	for (i = 0; i < 10; i++) {
		if ((a = srv("tcp!registry.9gridchan.org!6675")) != NULL)
			break;
		sleep(5);
	}
	if (a == NULL)
		return NULL;

	a->r = ctxregistryR;
	c9version((C9ctx*)a, &tag, Msize);
	wrsend(a);
	while (c9proc((C9ctx*)a) == 0 && a->r == ctxregistryR)
		wrsend(a);

	return a;
}

static int
chatrw(C9aux *a)
{
	struct timeval t;
	fd_set r, e;
	int n, sz;
	C9tag tag;
	C9ctx *ctx;
	char *s;

	FD_ZERO(&r);
	FD_SET(a->f, &r);
	FD_SET(0, &r);
	FD_ZERO(&e);
	FD_SET(a->f, &e);
	FD_SET(0, &e);
	memset(&t, 0, sizeof(t));
	t.tv_sec = 10;
	for (;;) {
		errno = 0;
		if ((n = select(a->f + 1, &r, NULL, &e, &t)) < 0 || FD_ISSET(a->f, &e) || FD_ISSET(0, &e)) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		break;
	}

	ctx = (C9ctx*)a;
	if (FD_ISSET(a->f, &r)) {
		c9proc(ctx);
	} else if (FD_ISSET(0, &r)) {
		s = (char*)a->rdbuf;
		sz = sprintf(s, "%s → ", nick);
		for (;;) {
			if ((n = read(0, s+sz, sizeof(a->rdbuf)-sz)) > 0)
				sz += n;
			else
				exit(0);
			if (s[sz-1] != '\n'){
				s[sz-1] = '\n';
			}else{
				c9write(ctx, &tag, Chatfid, 0, s, sz);
				break;
			}
		}
	} else {
		const char *path[] = {NULL};
		c9walk(ctx, &tag, Rootfid, Rootfid, path);
	}

	return 0;
}

int
main(int argc, char **argv)
{
	struct termios t;
	C9aux *a;
	int c, noecho;

	if (noecho && tcgetattr(0, &t) == 0) {
		t.c_lflag &= ~ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &t);
	}

	for (;;) {
		if ((a = registry()) == NULL)
			return 1;
		while (chatrw(a) == 0 && wrsend(a) == 0);
		if (a->flags & (Disconnected|Error)) {
			a->flags &= ~(Disconnected|Error);
			skipuntil = chatoff;
			chatoff = 0;
			free(a);
		} else {
			printf("exiting\n");
			exit(1);
		}
	}

	return 0;
}
