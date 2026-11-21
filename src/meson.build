diary_src = [
  gresources,

  'headerbuttons.c',
  'main.c',
  'utils.c',
  'window.c',

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
