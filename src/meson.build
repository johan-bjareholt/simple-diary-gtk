diary_src = [
  gresources,

  'entry.c',
  'entry_view.c',
  'entry_edit.c',
  'entry_list.c',
  'entry_listing.c',
  'main.c',
  'image_picker.c',
  'notification.c',
  'utils.c',
  'md2html.c',
  'settings.c',
  'window.c',
]

diary_deps = [
  gtk_dep,
  md4c_dep,
  md4c_html_dep,
  webkitgtk_dep
]

executable(
  'simple-diary', diary_src,
  dependencies: diary_deps,
  install: true,
  gui_app: true,
)
