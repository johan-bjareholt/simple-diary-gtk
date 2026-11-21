diary_src = [
  gresources,

  'main.c',
  'utils.c',

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
