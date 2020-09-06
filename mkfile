<$PLAN9/src/mkhdr

BIN=/usr/local/bin

TARG=x9dev
CC=cc
OFILES=\
	x9dev.$O\
	plan9port.$O\
	shadowfb.$O\
# Xext \
	shape.$O\
	xres.$O\
# Xi \
	allowev.$O\
	chgdctl.$O\
	chgfctl.$O\
	chgkbd.$O\
	chgkmap.$O\
	chgprop.$O\
	chgptr.$O\
	closedev.$O\
	devbell.$O\
	exevents.$O\
	extinit.$O\
	getbmap.$O\
	getdctl.$O\
	getfctl.$O\
	getfocus.$O\
	getkmap.$O\
	getmmap.$O\
	getprop.$O\
	getselev.$O\
	getvers.$O\
	grabdev.$O\
	grabdevb.$O\
	grabdevk.$O\
	gtmotion.$O\
	listdev.$O\
	opendev.$O\
	queryst.$O\
	selectev.$O\
	sendexev.$O\
	setbmap.$O\
	setdval.$O\
	setfocus.$O\
	setmmap.$O\
	setmode.$O\
	stubs.$O\
	ungrdev.$O\
	ungrdevb.$O\
	ungrdevk.$O\
# config \
	config.$O\
# dix \
	atom.$O\
	colormap.$O\
	cursor.$O\
	devices.$O\
	dispatch.$O\
	dixfonts.$O\
	dixutils.$O\
	enterleave.$O\
	eventconvert.$O\
	events.$O\
	extension.$O\
	gc.$O\
	getevents.$O\
	globals.$O\
	glyphcurs.$O\
	grabs.$O\
	initatoms.$O\
	inpututils.$O\
	main.$O\
	pixmap.$O\
	privates.$O\
	ptrveloc.$O\
	property.$O\
	region.$O\
	registry.$O\
	resource.$O\
	selection.$O\
	stubmain.$O\
	swaprep.$O\
	swapreq.$O\
	tables.$O\
	touch.$O\
	window.$O\
# fb \
	fballpriv.$O\
	fbarc.$O\
	fbbits.$O\
	fbblt.$O\
	fbbltone.$O\
	fbcmap_mi.$O\
	fbcopy.$O\
	fbfill.$O\
	fbfillrect.$O\
	fbfillsp.$O\
	fbgc.$O\
	fbgetsp.$O\
	fbglyph.$O\
	fbimage.$O\
	fbline.$O\
	fboverlay.$O\
	fbpict.$O\
	fbpixmap.$O\
	fbpoint.$O\
	fbpush.$O\
	fbscreen.$O\
	fbseg.$O\
	fbsetsp.$O\
	fbsolid.$O\
	fbtrap.$O\
	fbutil.$O\
	fbwindow.$O\
# mi \
	miarc.$O\
	mibitblt.$O\
	micmap.$O\
	micopy.$O\
	midash.$O\
	midispcur.$O\
	mieq.$O\
	miexpose.$O\
	mifillarc.$O\
	mifillrct.$O\
	migc.$O\
	miglblt.$O\
	miinitext.$O\
	mioverlay.$O\
	mipointer.$O\
	mipoly.$O\
	mipolypnt.$O\
	mipolyrect.$O\
	mipolyseg.$O\
	mipolytext.$O\
	mipushpxl.$O\
	miscrinit.$O\
	misprite.$O\
	mivaltree.$O\
	miwideline.$O\
	miwindow.$O\
	mizerarc.$O\
	mizerclip.$O\
	mizerline.$O\
# miext/damage \
	damage.$O\
# miext/shadow \
	shadow.$O\
# os \
	WaitFor.$O\
	access.$O\
	auth.$O\
	connection.$O\
	io.$O\
	log.$O\
	mitauth.$O\
	oscolor.$O\
	osinit.$O\
	utils.$O\
	xprintf.$O\
	xstrans.$O\
# render \
	animcur.$O\
	filter.$O\
	glyph.$O\
	matrix.$O\
	miindex.$O\
	mipict.$O\
	mirect.$O\
	mitrap.$O\
	mitri.$O\
	picture.$O\
	render.$O\

HFILES=\
	dix-config.h

<$PLAN9/src/mkone

CFLAGS=-c -D_POSIX_SOURCE -D_BSD_EXTENSION -D_SUSV2_SOURCE -DSHADOWFB \
	-DHAVE_DIX_CONFIG_H -DPLAN9 -DT$objtype -I. -I../../include \
	-I../../fb -I../../mi -I../../miext/damage -I../../miext/shadow -I../../randr \
	-I../../render -I../../Xext -I../../Xi \
	-I/opt/X11/include -I/opt/X11/include/pixman-1  \
	-I/opt/X11/include/xorg \
	-I/Users/halfwit/Library/Plan9/include #-DDEBUG

x9dev.$O: x9dev.c keymap.h x9dev.h
plan9port.$O: plan9port.c x9dev.h
shadowfb.$O: sfbmodule.c shadowfb.h

%.$O: ../../Xext/%.c
	$CC  $CFLAGS -I../xfree86/dixmods/extmod ../../Xext/$stem.c

%.$O: ../../Xi/%.c
	$CC  $CFLAGS ../../Xi/$stem.c

%.$O: ../../config/%.c
	$CC  $CFLAGS ../../config/$stem.c

%.$O: ../../dix/%.c
	$CC  $CFLAGS ../../dix/$stem.c

%.$O: ../../fb/%.c
	$CC  $CFLAGS ../../fb/$stem.c

%.$O: ../../mi/%.c
	$CC  $CFLAGS ../../mi/$stem.c

%.$O: ../../miext/damage/%.c
	$CC  $CFLAGS -I../../miext/cw ../../miext/damage/$stem.c

%.$O: ../../miext/shadow/%.c
	$CC  $CFLAGS ../../miext/shadow/$stem.c

%.$O: ../../os/%.c
	$CC  $CFLAGS ../../os/$stem.c

%.$O: ../../render/%.c
	$CC $CFLAGS ../../render/$stem.c

