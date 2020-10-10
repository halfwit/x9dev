srcs = [
    'x9dev.c',
    'keyboard.c',
    'mouse.c',
    'screen.c',
    '../../mi/miinitext.c',
    '../../mi/mi.h',
    'x9dev.h',
    'keymap.h',
]

cc_args = [
    '-Wno-missing-braces',
]

x9dev_server = executable(
    'x9dev',
    srcs,
    include_directories: inc, 
    dependencies: common_dep,
    link_with: [
        c9,
        libdraw,
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

install_man(configure_file(
    input: 'man/x9dev.man',
    output: 'x9dev.1',
    configuration: manpage_config,
))
