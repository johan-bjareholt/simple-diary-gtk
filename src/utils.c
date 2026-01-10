#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <string.h>

#include "utils.h"
#include "utils-rs.h"

void
utils_error_dialog(GtkWidget *parent, gchar *format, ...)
{
  gchar *message;
  va_list args;

  va_start (args, format);
  g_vasprintf (&message, format, args);
  va_end (args);

  rs_utils_error_dialog (parent, message);

  g_free (message);
}

// Keep this in C since it returns a pointer into the original string
gchar *
utils_get_file_extension (gchar *filepath)
{
  return strrchr (filepath, '.');
}
