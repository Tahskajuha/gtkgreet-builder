dependencies = [dependency('gtk4', version: '>=4.18'), dependency('json-c')]

gtk_layer_shell = dependency('gtk4-layer-shell-0', required: get_option('layershell'))
if gtk_layer_shell.found()
	dependencies += gtk_layer_shell
	add_project_arguments('-DLAYER_SHELL', language : 'c')
endif

executable(
		'gtkgreet-builder',
		[
			'main.c',
			'proto.c',
			'config.c',
			'actions.c',
			'window.c',
			'gtkgreet.c',
                        'uimodel.c',
                        'fbWidgets.c'
		],
		dependencies: dependencies,
		install: true)
