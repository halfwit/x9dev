srcs = [
    'plan9.c',
    'x9dev.c',
    'x9dev.h',
]

x9dev_server = executable(
    'x9dev',
    srcs,
    include_directories: inc,
    link_with: [
        libxserver_main,
        libxserver_fb,
        libxserver,
        libxserver_xkb_stubs,
        libxserver_glx,
        libglxvnd,
    ],
    install: true,
)

install_man(configure_file(
    input: 'man/x9dev.man',
    output: 'x9dev.1',
    configuration: manpage_config,
))
