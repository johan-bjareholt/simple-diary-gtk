project('simple-diary', 'c', meson_version: '>=0.49', warning_level: 2)

gnome = import('gnome')

gtk_dep = dependency('gtk4', version : '>=4.0')
handy_dep = dependency('libadwaita-1', version : '>=1.0')
md4c_dep = dependency('md4c')

add_global_link_arguments(['-Wall'], language: 'c')

app_id = 'com.bjareholt.johan.SimpleDiary'
summary = 'Simple and lightweight diary app'
name = 'Simple Diary'

subdir('res')
subdir('src')

if get_option('test').enabled()
  subdir('test')
endif
