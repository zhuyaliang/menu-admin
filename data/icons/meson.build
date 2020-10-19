apps_icons = [
  ['scalable', 'menu-admin.svg'],
]

foreach icon: apps_icons
  install_data(
    join_paths(icon[0], icon[1]),
    install_dir: join_paths(ma_datadir, 'icons', 'hicolor', icon[0], 'apps')
  )
endforeach
