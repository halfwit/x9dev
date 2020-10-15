libdraw_src = [
    'bytesperline.c',
    'init.c',
    'loadimage.c',
]

libdraw = static_library(
    'libdraw',
    libdraw_src,
    include_directories: inc,
    dependencies: common_dep,
)

install_data('draw.h', install_dir: xorgsdkdir)
