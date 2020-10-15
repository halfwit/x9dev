srcs = [
    'x9dev.c',
    'keyboard.c',
    'mouse.c',
    'screen.c',
    'x9dev.h',
    'keymap.h',
    '../../../mi/miinitext.c',
]

cc_args = [
    '-Wno-missing-braces',
]

x9dev_server = executable(
    'x9dev',
    srcs,
    include_directories: [
        inc,
        include_directories('../libdraw'),
    ],
    dependencies: [
        common_dep,
        dependency('xi', version: '>= 1.2.99.1'),
    ],
    link_with: [
        libdraw,
        libxserver_mi,
        libxserver_main,
        libxserver_fb,
        libxserver,
        libxserver_xkb_stubs,
        libxserver_glx,
        libglxvnd,
        libxserver_miext_shadow,
    ],
    install: true,
    c_args: cc_args,
)