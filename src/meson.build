gnome = import('gnome')

resources = gnome.compile_resources(
  'nautilus-resources',
  join_paths(
    'resources', 'menu-admin.gresource.xml'
  ),
  source_dir: 'resources',
  c_name: 'menu_admin',
  extra_args: '--generate-source'
)


sources = [
'main.c',
'app-menu.c',
'app-util.c',
'menu-window.c',
'menu-tool.c',
'menu-lockdown.c',
 resources,
]

executable('menu-admin',
  sources : sources,
  dependencies : [gtk_deps,menu_deps,dm_deps,act_deps],
  include_directories: top_srcdir,
  install : true,
  install_dir : get_option('bindir')
  )
