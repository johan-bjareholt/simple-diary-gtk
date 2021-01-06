subdir('gtk')
subdir('icons')

conf_data = configuration_data()
conf_data.set('app_id', app_id)
conf_data.set('summary', summary)
conf_data.set('name', name)

desktop_file = configure_file(
  input: files(app_id + '.desktop.in'),
  output: app_id + '.desktop',
  configuration: conf_data
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

appstream_file = configure_file(
  input: app_id + '.metainfo.xml.in',
  output: app_id + '.metainfo.xml',
  configuration: conf_data,
)


appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate', appstream_file]
  )
endif

install_data(
  appstream_file,
  install_dir: get_option('datadir') / 'metainfo',
)
