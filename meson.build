project('simple-diary', 'c',
  meson_version: '>=0.49',
  default_options: ['warning_level=1', 'werror=true'])

gnome = import('gnome')

gtk_dep = dependency('gtk4', version : '>=4.10')
adwaita_dep = dependency('libadwaita-1', version : '>=1.5')
gdkpixbuf_dep = dependency('gdk-pixbuf-2.0', version : '>=2.0')
gtkmdeditor_dep = dependency('gtkmdeditor')

add_global_link_arguments(['-Wall'], language: 'c')

app_id = 'com.bjareholt.johan.SimpleDiary'
summary = 'Simple and lightweight diary app'
name = 'Simple Diary'

subdir('res')
subdir('src')

if get_option('test').enabled()
  subdir('test')
endif
