# x9dev

__*THIS IS A WORK IN PROGRESS*__

x9dev is an x11 server which interacts with plan9's /dev filesystem.
This started as a fork of [equis](http://plan9.stanleylieber.com/pkg/386/equis-2012.01.11.tbz)

## Usage

`x9dev [-D]` should be ran in a chroot, where your plan9 /dev/draw has been imported.

## Building

Requires plan9port

```rc
# currently this builds against xorg-server 1.20
git clone https://github.com/halfwit/x9dev
mv x9dev /path/to/xorg-server-source/hw/
cd /path/to/xorg-server-source/

# Patch
git apply -p1 meson.patch

# Set up build system
meson build
meson configure build/ -Dbuild_x9dev=true
# Here you'd also tune to your system, see `meson configure build/` for a list of tunable options
# Build
ninja -C build

install -m644 build/hw/x9dev/src/x9dev /usr/local/bin/x9dev
```

## Extensions

There are currently no supported X11 extensions, though plans to implement them are in place. 
