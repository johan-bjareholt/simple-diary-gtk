#include <gtk/gtk.h>
#include <errno.h>

#include "window.h"
#include "utils.h"
#include "settings.h"

static void
init(GtkApplication *app)
{
  gchar *path = utils_get_diary_folder ();
  if (g_mkdir_with_parents (path, 0750) < 0) {
    gchar *msg = g_strdup_printf ("Failed to create diary folder at '%s' because:\n%s", path, strerror(errno));
    utils_error_dialog(msg);
    g_free (msg);
    return;
  }
  g_free (path);

  utils_apply_dark_mode ();

  GtkApplicationWindow *window;
  window = diary_window_new (app);
  gtk_widget_show(GTK_WIDGET (window));
}

int
main(int argc, char *argv[])
{
    GtkApplication *app;
    gtk_init (&argc, &argv);

    settings_init ();
    app = gtk_application_new ("com.bjareholt.johan.SimpleDiary", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (init), app);

    int status = g_application_run (G_APPLICATION (app), argc, argv);

    return status;
}
