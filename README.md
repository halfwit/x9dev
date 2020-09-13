# x9dev

__*THIS IS A WORK IN PROGRESS*__

x9dev is an x11 server which interacts with plan9's /dev filesystem.
This started as a fork of http://plan9.stanleylieber.com/pkg/386/equis-2012.01.11.tbz

## Usage

`x9dev [-w] [-x cmd] dir`

- `-w` uses the /dev's wsys for window creation and control
- `-x` will run the cmd on connection, usually an x11 program in a full screen

Example:

```rc
# Import a remote /dev
9pfuse -a dev 192.168.0.10 /tmp/9/dev
x9dev /tmp/9/dev
```

## Building

Requires plan9port

```rc
# currently this builds against xorg-server 1.20
git clone https://github.com/halfwit/x9dev
mv x9dev /path/to/xorg-server-source/hw/
cd /path/to/xorg-server-source/

# Add our entry to the meson build
echo '
if build_x9dev
    subdir('x9dev')
endif
' >> hw/meson.build

meson build
meson configure build/ -Dbuild_x9dev=true
ninja -C build -j 8 # or however many processors you have

install -m644 hw/x9dev /usr/local/bin/x9dev
```
