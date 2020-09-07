srcs = [
    'x9dev.c',
    'plan9.c',
    '../../mi/miinitext.c',
]

x9dev_dep = [
    common_dep,
]

x9dev_server = executable(
    'x9dev',
    srcs,
    include_directories: [
	inc,
	include_directories('/usr/local/plan9/include')
    ], 
    dependencies: x9dev_dep,
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
