project('simple-diary', 'c', meson_version: '>=0.49', warning_level: 2)

gnome = import('gnome')

gtk_dep = dependency('gtk+-3.0', version : '>=3.22')
md4c_dep = dependency('md4c')
md4c_html_dep = dependency('md4c-html')
webkitgtk_dep = dependency('webkit2gtk-4.0')

add_global_link_arguments(['-Wall'], language: 'c')

app_id = 'com.bjareholt.johan.SimpleDiary'
summary = 'Simple and lightweight diary app'
name = 'Simple Diary'

subdir('res')
subdir('src')

if get_option('test').enabled()
  subdir('test')
endif
