draw_src = [
    'alloc.c',
    'arith.c',
    'badrect.c',
    'buildfont.c',
    'bytesperline.c',
    'chan.c',
    'defont.c',
    'draw.c',
    'freesubfont.c',
    'getdefont.c',
    'init.c',
    'loadimage.c',
    'openfont.c',
    'subfont.c',
    'subfontcache.c',
    'window.c',
]

draw = static_library(
    'draw',
    draw_src,
    include_directories: inc,
    dependencies: common_dep,
    install: true,
)

install_data('draw.h', install_dir: xorgsdkdir)
