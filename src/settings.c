#include <glib.h>

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
    g_printerr ("Settings file is corrypted: %s\n", err->message);
    exit (EXIT_FAILURE);
  }

  return diary_folder;
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
      gchar * diary_folder = g_strdup_printf("%s/Documents/Diary", home);
      g_key_file_set_string (keyfile, "Notebooks", "Default", diary_folder);
      write = TRUE;
      g_free (diary_folder);
    }
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
