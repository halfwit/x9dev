# x9dev

__*THIS IS A WORK IN PROGRESS*__

 - this is still in the design phase, though some code exists; no draw calls have been written yet.
 
x9dev is an x11 server which interacts with plan9's /dev filesystem.
This started as a fork of http://plan9.stanleylieber.com/pkg/386/equis-2012.01.11.tbz

## Usage

`x9dev [-u] [-s screen]`

- `-u` creates a unique session
- `-s screen` attaches to a named session

`x9dev` is meant to be called via an external dial implementation. For example, with x9srv on a POSIX system

`tlsclient 'tcp!192.168.1.2!17019' x9dev`

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
ninja -C build -j 8 # or however many processors you have

install -m644 hw/x9dev /usr/local/bin/x9dev
```

## Extensions

There are currently no supported X11 extensions, though plans to implement them are in place. 
