subdir('plan9')

srcs = [
    'x9dev.c',
    '../../mi/miinitext.c',
]

x9dev_server = executable(
    'x9dev',
    srcs,
    include_directories: inc, 
    dependencies: common_dep,
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
)

install_man(configure_file(
    input: 'man/x9dev.man',
    output: 'x9dev.1',
    configuration: manpage_config,
))
