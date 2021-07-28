subdir('draw')

srcs = [
    'x9dev.c',
    'keyboard.c',
    'mouse.c',
    'screen.c',
    'x9dev.h',
    'keymap.h',
    '../../mi/miinitext.c',
]

cc_args = [
    '-Wno-missing-braces',
]

x9dev_server = executable('x9dev',
    srcs,
    include_directories: inc, 
    dependencies: common_dep, 
    link_with: [
        draw,
        libxserver_mi,
        libxserver_main,
        libxserver_fb,
        libxserver,
        libxserver_xkb_stubs,
        libxserver_xi_stubs,
        libxserver_glx,
        libglxvnd,
        libxserver_miext_shadow,
    ],
    c_args: cc_args,
    install: true,
)

install_man(configure_file(
    input: 'man/x9dev.man',
    output: 'x9dev.1',
    configuration: manpage_config,
))
