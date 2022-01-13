#include <glib.h>

#include "settings.h"

static gchar *keyfile_path = NULL;
static GKeyFile *keyfile = NULL;

gchar *
settings_get_diary_folder (void)
{
  gchar *diary_folder;
  GError *err = NULL;

  g_assert_nonnull (keyfile);

  diary_folder = g_key_file_get_string (keyfile, "Notebooks", "Default", &err);
  if (err != NULL) {
    g_printerr ("Settings file is corrupted: %s\n", err->message);
    exit (EXIT_FAILURE);
  }

  return diary_folder;
}

ColorScheme
settings_get_color_scheme (void)
{
  ColorScheme color_scheme;
  gchar *color_scheme_str;

  g_assert_nonnull (keyfile);

  color_scheme_str = g_key_file_get_string (keyfile, "Appearance", "ColorScheme", NULL);
  if (color_scheme_str == NULL) {
    g_print ("Could not find Appearance.ColorScheme, assuming default\n");
    g_strdup ("default");
  }

  if (g_strcmp0 (color_scheme_str, "light") == 0) {
    color_scheme = COLOR_SCHEME_LIGHT;
  } else if (g_strcmp0 (color_scheme_str, "default") == 0) {
    color_scheme = COLOR_SCHEME_DEFAULT;
  } else if (g_strcmp0 (color_scheme_str, "dark") == 0) {
    color_scheme = COLOR_SCHEME_DARK;
  } else {
    g_print ("Invalid color scheme '%s' in settings, assuming default\n",
        color_scheme_str);
    color_scheme = COLOR_SCHEME_DEFAULT;
  }

  return color_scheme;
}

void
settings_set_color_scheme (ColorScheme color_scheme)
{
  GError *err = NULL;
  gchar *color_scheme_str;

  g_assert_nonnull (keyfile);

  switch (color_scheme) {
    case COLOR_SCHEME_LIGHT:
      color_scheme_str = "light";
      break;
    case COLOR_SCHEME_DEFAULT:
      color_scheme_str = "default";
      break;
    case COLOR_SCHEME_DARK:
      color_scheme_str = "dark";
      break;
  }

  g_print ("Saving color scheme preference: %s\n", color_scheme_str);

  g_key_file_set_string (keyfile, "Appearance", "ColorScheme",
      color_scheme_str);

  if (!g_key_file_save_to_file (keyfile, keyfile_path, &err)) {
    g_warning ("Error saving settings file: %s", err->message);
    exit (EXIT_FAILURE);
  }
}

/* If some required setting is missing, this function will return TRUE */
static gboolean
settings_load_default_config (void)
{
  gboolean write = FALSE;
  char *home = getenv("HOME");
  GError *err = NULL;

  // Notebooks.Default
  g_key_file_get_string (keyfile, "Notebooks", "Default", &err);
  if (err != NULL) {
    if (g_error_matches (err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) ||
        g_error_matches (err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND)) {
      gchar * diary_folder = g_strdup_printf("%s/Documents/SimpleDiary", home);
      g_key_file_set_string (keyfile, "Notebooks", "Default", diary_folder);
      write = TRUE;
      g_free (diary_folder);
    } else {
      g_printerr ("Settings file is corrupted: %s\n", err->message);
      exit (EXIT_FAILURE);
    }
    g_clear_error (&err);
  }

  /* Appearance.DarkMode */
  g_key_file_get_boolean (keyfile, "Appearance", "DarkMode", &err);
  if (err != NULL) {
    if (g_error_matches (err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) ||
        g_error_matches (err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND)) {
      g_key_file_set_boolean (keyfile, "Appearance", "DarkMode", FALSE);
      write = TRUE;
    } else {
      g_printerr ("Settings file is corrupted: %s\n", err->message);
      exit (EXIT_FAILURE);
    }
    g_clear_error (&err);
  }

  return write;
}

void
settings_init (void)
{
  gchar *config_folder;
  char *home = getenv("HOME");
  GError *err = NULL;

  g_assert_null (keyfile);

  config_folder = g_strdup_printf ("%s/.config/simple-diary", home);
  if (g_mkdir_with_parents (config_folder, 0750) < 0) {
    g_printerr ("Failed to create config folder: %s\n", err->message);
    exit (EXIT_FAILURE);
  }

  keyfile = g_key_file_new ();
  keyfile_path = g_strdup_printf ("%s/settings.ini", config_folder);

  g_free (config_folder);

  if (!g_key_file_load_from_file (keyfile, keyfile_path, 0, &err)) {
    if (g_error_matches (err, G_FILE_ERROR, G_FILE_ERROR_NOENT)) {
      // TODO: create default
      g_print ("Writing new config\n");
      g_clear_error (&err);
    } else {
      g_printerr ("Failed to load config: %s\n", err->message);
      exit (EXIT_FAILURE);
    }
  }

  if (settings_load_default_config ()) {
    /* settings file has changed, needs to be written */
    g_print ("Writing config at '%s'\n", keyfile_path);
    if (!g_key_file_save_to_file (keyfile, keyfile_path, &err)) {
      g_warning ("Error saving settings file: %s", err->message);
      exit (EXIT_FAILURE);
    }
  }
}
