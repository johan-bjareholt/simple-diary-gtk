#include <gtk/gtk.h>

#include "utils.h"

void
utils_error_dialog(gchar *message)
{
  GtkWidget *dialog =
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
  char *home = getenv("HOME");
  return g_strdup_printf("%s/Documents/Diary", home);
}

/*TODO: Move this to entry class? */
gchar *
utils_get_photos_folder (gchar *basename)
{
  gchar *diary_folder;
  gchar *photos_folder;

  diary_folder = utils_get_diary_folder ();
  photos_folder = g_strdup_printf ("%s/photos/%s", diary_folder, basename);

  g_free (diary_folder);

  if (g_mkdir_with_parents (photos_folder, 0750) < 0) {
    gchar *msg = g_strdup_printf ("Failed to create folder at '%s' because: %s\n", photos_folder, strerror(errno));
    utils_error_dialog(msg);
    g_free (msg);
    g_free (photos_folder);
    photos_folder = NULL;
  }

  return photos_folder;
}

gchar *
utils_get_file_extension (gchar *filepath)
{
  return strrchr (filepath, '.');
}
