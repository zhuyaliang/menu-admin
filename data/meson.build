subdir('icons')
i18n = import('i18n')

appdata = 'org.menuadmin.gschema.xml'
i18n.merge_file(
  input: appdata + '.in',
  output: appdata,
  po_dir: join_paths(meson.source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('prefix'), 'share/glib-2.0/schemas')
)  
