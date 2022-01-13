#include <gtk/gtk.h>
#include <adwaita.h>
#include <errno.h>

#include "window.h"
#include "utils.h"
#include "settings.h"

static void
init(GtkApplication *app)
{
  GtkApplicationWindow *window;
  gchar *diary_path;

  diary_path = utils_get_diary_folder ();
  if (g_mkdir_with_parents (diary_path, 0750) < 0) {
    gchar *msg = g_strdup_printf (
        "Failed to create diary folder at '%s' because:\n%s",
        diary_path, strerror(errno));
    utils_error_dialog(msg);
    g_free (msg);
    return;
  }
  g_free (diary_path);

  utils_apply_dark_mode ();

  window = diary_window_new (app);
  gtk_widget_show(GTK_WIDGET (window));
}

int
main(int argc, char *argv[])
{
    AdwApplication *app;

    gtk_init ();
    adw_init ();

    settings_init ();
    app = adw_application_new ("com.bjareholt.johan.SimpleDiary", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (init), app);

    int status = g_application_run (G_APPLICATION (app), argc, argv);

    return status;
}
