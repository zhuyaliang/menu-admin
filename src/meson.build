gnome = import('gnome')

resources = gnome.compile_resources(
  'nautilus-resources',
  join_paths(
    'resources', 'nautilus.gresource.xml'
  ),
  source_dir: 'resources',
  c_name: 'nautilus',
  extra_args: '--generate-source'
)


sources = [
'main.c',
'app-menu.c',
'app-util.c',
'menu-window.c',
 resources,
]

executable('menu-admin',
  sources : sources,
  dependencies : [gtk_deps,md_deps,polkit_deps],
  include_directories: top_srcdir,
  install : true,
  install_dir : get_option('bindir')
  )
