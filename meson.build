project('simple-diary',
  meson_version: '>=1',
  default_options: ['warning_level=1', 'werror=true'])

app_id = 'com.bjareholt.johan.SimpleDiary'
summary = 'Simple and lightweight diary app'
name = 'Simple Diary'

subdir('res')

if get_option('test').enabled()
  subdir('test')
endif
