diary_src = [
  gresources,

  'entry.c',
  'headerbuttons.c',
  'main.c',
  'notification.c',
  'utils.c',
  'md2html.c',
  'settings.c',
  'window.c',

  'widgets/entry_browser.c',
  'widgets/entry_view.c',
  'widgets/entry_edit.c',
  'widgets/entry_list.c',
  'widgets/entry_listing.c',
  'widgets/image_picker.c',
  'widgets/settings_view.c',
]

diary_deps = [
  gtk_dep,
  handy_dep,
  md4c_dep,
  md4c_html_dep,
  webkitgtk_dep
]


executable(
  'simple-diary', diary_src,
  dependencies: diary_deps,
  install: true,
  gui_app: true,
  link_args: '-rdynamic',
)

executable('adaptive', ['adaptive.c'], dependencies : diary_deps, gui_app : true)
