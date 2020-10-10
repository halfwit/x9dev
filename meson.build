srcs = [
    'x9dev.c',
    'draw.c',
    'c9/c9.c',
    'c9/c9.h',
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
    object: '/usr/local/plan9/lib/libdraw.a',
    link_with: [
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
