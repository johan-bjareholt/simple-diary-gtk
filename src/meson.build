diary_src = [
  gresources,

  'headerbuttons.c',
  'main.c',
  'notification.c',
  'utils.c',
  'window.c',

  'widgets/entry_browser.c',
  'widgets/entry_list.c',
  'widgets/entry_listing.c',
  'widgets/settings_view.c',
]

diary_deps = [
  gtk_dep,
  adwaita_dep,
  gdkpixbuf_dep,
  rust_dep,
]

executable(
  'simple-diary', diary_src,
  dependencies: diary_deps,
  install: true,
  gui_app: true,
  link_args: '-rdynamic',
)
