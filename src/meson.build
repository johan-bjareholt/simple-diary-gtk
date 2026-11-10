diary_src = [
  gresources,

  'headerbuttons.c',
  'main.c',
  'notification.c',
  'utils.c',
  'window.c',

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
  link_args: '-rdynamic',
)
