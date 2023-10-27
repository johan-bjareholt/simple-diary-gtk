#include <glib.h>
#include <gio/gio.h>
#include <glib/gstdio.h>


#include "settings.h"

static gchar *keyfile_path = NULL;
static GKeyFile *keyfile = NULL;

/* config home paths */

static gchar *
xdg_config_dir(void)
{
  gchar *dir = NULL;
  const char *xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home) {
    dir = g_strdup_printf ("%s/simple-diary", xdg_config_home);
  }
  return dir;
}

static gchar *
home_config_dir(void)
{
  const char *home = getenv("HOME");
  return g_strdup_printf ("%s/.config/simple-diary", home);
}

static gchar *
config_dir(void)
{
  gchar *dir = NULL;

  dir = xdg_config_dir();
  if (dir == NULL) {
    dir = home_config_dir();
  }

  return dir;
}

/* data dir paths */

static gchar *
xdg_data_dir(void)
{
  gchar *dir = NULL;
  const char *xdg_config_home = getenv("XDG_DATA_HOME");
  if (xdg_config_home) {
    dir = g_strdup_printf ("%s/SimpleDiary", xdg_config_home);
  }
  return dir;
}

static gchar *
home_data_dir(void)
{
  const char *home = getenv("HOME");
  return g_strdup_printf ("%s/Documents/SimpleDiary", home);
}

static gchar *
default_data_dir(void)
{
  gchar *dir = NULL;

  dir = xdg_data_dir();
  if (dir == NULL) {
    dir = home_data_dir();
  }

  return dir;
}

/* migrations */

/* This migration is made because:
 * When the flatpak was first made, it wrote the config to ~/.config and the
 * data to ~/Documents. This required extra permissions in the flatpak.
 * The migration will move the data to its respective XDG dirs, so it's reading
 * and writing in a folder which fits the flatpak security model.
 * Once this migration has existed for a year or more, I think we should remove
 * it altogether and then remove the permission in the flatpak to write in the
 * users home folder.
 *
 * TODO: This code was written 2022-11-06, so at least a year after this date
 * we should consider fixing this */
static void
migrate_datadir(void)
{
  g_autofree gchar *new_str = NULL;
  g_autofree gchar *old_str = NULL;

  new_str = default_data_dir();
  old_str = home_data_dir();

  if (!g_file_test(new_str, G_FILE_TEST_EXISTS) &&
      g_file_test(old_str, G_FILE_TEST_EXISTS)) {
    int ret;
    g_autofree gchar *cmd = NULL;
    g_autofree gchar *old_config_dir = NULL;
    g_autofree gchar *old_config = NULL;

    g_print("Migrating data from %s to %s\n", old_str, new_str);
    cmd = g_strdup_printf("mv %s %s", old_str, new_str);
    errno = 0;
    ret = system(cmd);
    if (ret != 0) {
        g_printerr("Failed to migrate datadir: %s\n", strerror(errno));
        exit(1);
    }

    old_config_dir = home_config_dir();
    old_config = g_strdup_printf("%s/settings.ini", old_config_dir);

    g_unlink(old_config);
  }
}

/* ... */

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
  g_autofree gchar *color_scheme_str;

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
  GError *err = NULL;
  g_autofree gchar *notebook = NULL;

  // Notebooks.Default
  notebook = g_key_file_get_string (keyfile, "Notebooks", "Default", &err);
  if (err != NULL) {
    if (g_error_matches (err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND) ||
        g_error_matches (err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND)) {
      g_autofree gchar *diary_folder = default_data_dir();
      g_key_file_set_string (keyfile, "Notebooks", "Default", diary_folder);
      write = TRUE;
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
  g_autofree gchar *config_folder = NULL;
  GError *err = NULL;

  g_assert_null (keyfile);

  migrate_datadir();

  config_folder = config_dir();
  if (g_mkdir_with_parents (config_folder, 0750) < 0) {
    g_printerr ("Failed to create config folder: %s\n", err->message);
    exit (EXIT_FAILURE);
  }

  keyfile = g_key_file_new ();
  keyfile_path = g_strdup_printf ("%s/settings.ini", config_folder);

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
