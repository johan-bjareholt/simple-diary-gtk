#pragma once

void settings_init (void);

gchar * settings_get_diary_folder (void);

gboolean settings_get_dark_mode (void);
void settings_set_dark_mode (gboolean enabled);
