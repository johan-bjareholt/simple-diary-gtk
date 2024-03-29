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
  g_print("Diary folder is '%s'\n", diary_path);
  if (g_mkdir_with_parents (diary_path, 0750) < 0) {
    gchar *msg = g_strdup_printf (
        "Failed to create diary folder at '%s' because:\n%s\n",
        diary_path, strerror(errno));
    g_printerr(msg);
    g_free (msg);
    return;
  }
  g_free (diary_path);

  utils_apply_color_scheme ();

  window = diary_window_new (app);
  gtk_widget_set_visible(GTK_WIDGET (window), TRUE);
}

int
main(int argc, char *argv[])
{
    AdwApplication *app;

    gtk_init ();
    adw_init ();

    settings_init ();
    app = adw_application_new ("com.bjareholt.johan.SimpleDiary", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (init), app);

    int status = g_application_run (G_APPLICATION (app), argc, argv);

    return status;
}
