#pragma once

#include <gtk/gtk.h>
#include <glib.h>

void rs_utils_error_dialog(GtkWidget *parent, gchar *message);

gchar* utils_get_diary_folder(void);
gchar* utils_get_photos_folder(gchar *basename, gboolean absolute);
void utils_apply_color_scheme(void);
