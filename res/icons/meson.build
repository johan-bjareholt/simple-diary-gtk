icon = '@0@.svg'.format(app_id)

scalable_dir = 'hicolor' / 'scalable' / 'apps'
install_data(
  icon,
  install_dir: get_option('datadir') / 'icons' / scalable_dir,
)

symbolic_dir = join_paths('hicolor', 'symbolic', 'apps')
install_data(
  icon,
  install_dir: get_option('datadir') / 'icons' / symbolic_dir,
  rename: '@0@-symbolic.svg'.format(app_id)
)
