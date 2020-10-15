libdraw_src = [
    'arith.c',
    'buildfont.c',
    'bytesperline.c',
    'chan.c',
    'defont.c',
    'draw.c',
    'getdefont.c',
    'init.c',
    'loadimage.c',
    'openfont.c',
]

libdraw = static_library(
    'libdraw',
    libdraw_src,
    include_directories: inc,
    dependencies: common_dep,
)

install_data('draw.h', install_dir: xorgsdkdir)
