#include <gtk/gtk.h>
#include <adwaita.h>
#include <glib/gprintf.h>
#include <errno.h>

#include "utils.h"
#include "window.h"
#include "settings.h"

static void
error_dialog_cb (AdwMessageDialog *dialog,
                 GAsyncResult     *result,
                 gpointer          user_data)
{
  gtk_window_destroy (GTK_WINDOW (dialog));
}

void
utils_error_dialog(GtkWidget *parent, gchar *format, ...)
{
  AdwDialog *dialog;
  gchar *message;
  va_list args;

  va_start (args, format);
  g_vasprintf (&message, format, args);
  g_printerr("Error: %s", message);

  dialog = adw_alert_dialog_new ("Error", message);


  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                    "close", "_Close",
                                    NULL);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog), "close", ADW_RESPONSE_DESTRUCTIVE);

  adw_alert_dialog_choose (ADW_ALERT_DIALOG (dialog), parent, NULL, (GAsyncReadyCallback) error_dialog_cb, NULL);
}

gchar *
utils_get_diary_folder (void)
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
    GtkWindow *window;
    window = GTK_WINDOW (diary_window_get_instance ());
    utils_error_dialog(GTK_WIDGET (window), msg);
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
utils_apply_color_scheme (void)
{
  AdwApplication *app;
  AdwStyleManager *style_mgr;
  ColorScheme color_scheme;

  color_scheme = settings_get_color_scheme ();

  app = ADW_APPLICATION (g_application_get_default ());
  style_mgr = adw_application_get_style_manager (app);

  switch (color_scheme) {
    case COLOR_SCHEME_LIGHT:
      g_object_set (style_mgr, "color-scheme", ADW_COLOR_SCHEME_PREFER_LIGHT, NULL);
      break;
    case COLOR_SCHEME_DEFAULT:
      g_object_set (style_mgr, "color-scheme", ADW_COLOR_SCHEME_DEFAULT, NULL);
      break;
    case COLOR_SCHEME_DARK:
      g_object_set (style_mgr, "color-scheme", ADW_COLOR_SCHEME_PREFER_DARK, NULL);
      break;
  }
}
