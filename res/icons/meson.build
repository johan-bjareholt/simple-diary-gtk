logo = 'logo.svg'

scalable_dir = 'hicolor' / 'scalable' / 'apps'
install_data(
  logo,
  install_dir: get_option('datadir') / 'icons' / scalable_dir,
  rename: '@0@.svg'.format(app_id)
)

symbolic_dir = join_paths('hicolor', 'symbolic', 'apps')
install_data(
  logo,
  install_dir: get_option('datadir') / 'icons' / symbolic_dir,
  rename: '@0@-symbolic.svg'.format(app_id)
)
