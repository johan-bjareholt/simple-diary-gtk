diary_src = [
  gresources,

  'entry.c',
  'headerbuttons.c',
  'main.c',
  'notification.c',
  'utils.c',
  'settings.c',
  'window.c',

  'widgets/entry_browser.c',
  'widgets/entry_view.c',
  'widgets/entry_edit.c',
  'widgets/entry_list.c',
  'widgets/entry_listing.c',
  'widgets/entry_name_dialog.c',
  'widgets/entry_rename_dialog.c',
  'widgets/entry_delete_dialog.c',
  'widgets/image_picker.c',
  'widgets/settings_view.c',
]

diary_deps = [
  gtk_dep,
  adwaita_dep,
  gdkpixbuf_dep,
  gtkmdview_dep,
]

executable(
  'simple-diary', diary_src,
  dependencies: diary_deps,
  install: true,
  gui_app: true,
  link_args: '-rdynamic',
)
