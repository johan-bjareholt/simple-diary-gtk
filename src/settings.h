#pragma once

enum _ColorScheme {
    COLOR_SCHEME_LIGHT,
    COLOR_SCHEME_DEFAULT,
    COLOR_SCHEME_DARK,
};

typedef enum _ColorScheme ColorScheme;

void settings_init (void);

gchar * settings_get_diary_folder (void);

ColorScheme settings_get_color_scheme (void);
void settings_set_color_scheme (ColorScheme color_scheme);
