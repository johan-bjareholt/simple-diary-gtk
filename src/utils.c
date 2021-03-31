#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <errno.h>

#include "utils.h"
#include "settings.h"

void
utils_error_dialog(gchar *format, ...)
{
  GtkWidget *dialog;
  gchar *message;
  va_list args;

  va_start (args, format);
  g_vasprintf (&message, format, args);
  dialog =
    gtk_message_dialog_new (NULL,
                            0,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE,
                            message);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
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

void
utils_apply_dark_mode (void)
{
  GtkSettings *settings;
  gboolean dark_mode_enabled;

  settings = gtk_settings_get_default ();
  dark_mode_enabled = settings_get_dark_mode ();

  g_object_set (settings, "gtk-application-prefer-dark-theme", dark_mode_enabled, NULL);
}
