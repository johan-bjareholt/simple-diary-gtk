#include <gtk/gtk.h>
#include <adwaita.h>
#include <glib/gprintf.h>
#include <errno.h>

#include "utils.h"
#include "window.h"
#include "settings.h"

void
utils_error_dialog(gchar *format, ...)
{
  GtkWidget *dialog;
  GtkWindow *window;
  gchar *message;
  va_list args;

  /* TODO GTK4: Fix broken dialog */
  va_start (args, format);
  g_vasprintf (&message, format, args);

  window = GTK_WINDOW (diary_window_get_instance ());
  dialog =
    gtk_message_dialog_new (window,
                            GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE,
                            message);
  gtk_widget_show (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), window);


  g_signal_connect_swapped (dialog,
                            "response",
                            G_CALLBACK (gtk_window_destroy),
                            dialog);
}

gchar *
utils_get_diary_folder ()
{
  return settings_get_diary_folder ();
}

/*TODO: Move this to entry class? */
gchar *
utils_get_photos_folder (gchar *basename, gboolean absolute)
{
  gchar *diary_folder;
  gchar *photos_folder_absolute;

  diary_folder = utils_get_diary_folder ();
  photos_folder_absolute = g_strdup_printf ("%s/photos/%s",
                                            diary_folder, basename);
  g_free (diary_folder);

  if (g_mkdir_with_parents (photos_folder_absolute, 0750) < 0) {
    gchar *msg;

    msg = g_strdup_printf ("Failed to create folder at '%s' because: %s\n",
                           photos_folder_absolute, strerror(errno));
    utils_error_dialog(msg);
    g_free (msg);
    g_free (photos_folder_absolute);
    photos_folder_absolute = NULL;
  }

  if (absolute) {
    return photos_folder_absolute;
  } else {
    g_free (photos_folder_absolute);
    return g_strdup_printf ("photos/%s", basename);
  }
}

gchar *
utils_get_file_extension (gchar *filepath)
{
  return strrchr (filepath, '.');
}

static AdwColorScheme default_color_scheme = -1;

void
utils_apply_dark_mode (void)
{
  AdwApplication *app;
  AdwStyleManager *style_mgr;
  AdwColorScheme color_scheme;
  gboolean local_darkmode;

  local_darkmode = settings_get_dark_mode ();

  app = ADW_APPLICATION (g_application_get_default ());
  style_mgr = adw_application_get_style_manager (app);
  color_scheme = adw_style_manager_get_color_scheme (style_mgr);

  if (default_color_scheme == -1) {
    default_color_scheme = color_scheme;
  }

  if (local_darkmode) {
    g_object_set (style_mgr, "color-scheme", ADW_COLOR_SCHEME_PREFER_DARK, NULL);
    g_print ("locally set darkmode\n");
  } else {
    g_object_set (style_mgr, "color-scheme", default_color_scheme, NULL);
    g_print ("applying default color scheme\n");
  }
}
