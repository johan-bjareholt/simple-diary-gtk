subdir('gtk')
subdir('icons')

desktop_conf = configuration_data()
desktop_conf.set('icon', app_id)

desktop_file = configure_file(
  input: files(app_id + '.desktop.in'),
  output: app_id + '.desktop',
  configuration: desktop_conf
)

desktop_file_validate = find_program('desktop-file-validate', required: false)
if desktop_file_validate.found()
  test(
    'validate-desktop',
    desktop_file_validate,
    args: [
      desktop_file
    ]
  )
endif

install_data(
  desktop_file,
  install_dir: get_option('prefix') / 'share/applications'
)
