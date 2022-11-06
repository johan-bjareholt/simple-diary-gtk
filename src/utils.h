#pragma once

#include <glib.h>

void utils_error_dialog (gchar *message, ...);

gchar *utils_get_diary_folder (void);

gchar *utils_get_photos_folder (gchar *basename, gboolean absolute);

gchar *utils_get_file_extension (gchar *filepath);

void utils_apply_color_scheme (void);
