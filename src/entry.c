#include <gio/gio.h>
#include <stdio.h>
#include <glib/gstdio.h>

#include "entry.h"
#include "utils.h"

struct _Entry
{
  GObject parent;

  gchar *folder;
  gchar *filename;
};

G_DEFINE_TYPE (Entry, entry, G_TYPE_OBJECT);

typedef enum
{
  PROP_FOLDER = 1,
  PROP_FILENAME,
  PROP_BASENAME,
  PROP_PATH,
  NUM_PROPS,
} EntryProperties;

static GParamSpec *obj_properties[NUM_PROPS] = { NULL, };

gchar *
filename_to_basename (gchar *filename)
{
  gchar *basename;
  gchar *last_dot;

  basename = g_path_get_basename (filename);
  last_dot = strrchr (basename, '.');
  last_dot[0] = '\0';

  return basename;
}

gboolean
entry_rename_file (Entry *self, gchar *new_name)
{
  gchar *old_file_path;
  gchar *new_file_path;

  gchar *old_basename;
  gchar *new_basename;

  gchar *old_photos_path_full = NULL;
  gchar *new_photos_path_full = NULL;

  gchar *old_photos_path_short = NULL;
  gchar *new_photos_path_short = NULL;

  /* TODO: Validate that it ends with .md */

  /* get basenames */
  old_basename = filename_to_basename (self->filename);
  new_basename = filename_to_basename (new_name);

  /* move md file */
  old_file_path = g_strdup_printf ("%s/%s", self->folder, self->filename);
  new_file_path = g_strdup_printf ("%s/%s", self->folder, new_name);
  if (rename (old_file_path, new_file_path) < 0) {
    /* TODO: Exit gracefully, show popup */
    g_printerr ("Failed to rename diary file '%s': %s\n", old_basename, strerror(errno));
    exit(EXIT_FAILURE);
  }
  g_free (old_file_path);
  g_free (new_file_path);

  /* apply new name */
  g_print ("eh\n");
  g_object_set (self, "filename", new_name, NULL);

  /* move photos folder */
  old_photos_path_full = g_strdup_printf ("%s/photos/%s", self->folder, old_basename);
  new_photos_path_full = g_strdup_printf ("%s/photos/%s", self->folder, new_basename);
  if (rename (old_photos_path_full, new_photos_path_full) < 0) {
    // TODO: Exit gracefully, show popup
    if (errno == ENOENT) {
        goto out;
    }
    g_printerr ("Failed to rename photos folder for diary '%s': %s\n", old_basename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  /* Change photo links to new folder */
  old_photos_path_short = g_strdup_printf ("photos/%s", old_basename);
  new_photos_path_short = g_strdup_printf ("photos/%s", new_basename);
  GError *err = NULL;
  gchar *text = entry_read (self, &err);
  if (text == NULL) {
    // TODO: Exit gracefully, show popup
    g_printerr ("Failed to read diary entry '%s': %s\n", old_basename, err->message);
    exit(EXIT_FAILURE);
  }
  char **split = g_strsplit(text, old_photos_path_short, -1);
  g_free(text);
  text = g_strjoinv(new_photos_path_short, split);
  g_strfreev(split);
  if (!entry_write (self, text, NULL)) {
    // TODO: Exit gracefully, show popup
    g_printerr ("Failed to write diary entry '%s': %s\n", old_basename, err->message);
    exit(EXIT_FAILURE);
  }

out:
  g_free (old_photos_path_short);
  g_free (new_photos_path_short);

  g_free (old_photos_path_full);
  g_free (new_photos_path_full);

  g_free (old_basename);
  g_free (new_basename);

  return TRUE;
}

static void
entry_set_property (GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
  Entry *self = DIARY_ENTRY (object);

  switch ((EntryProperties) property_id) {
    case PROP_FOLDER:
      if (self->folder != NULL)
        g_free (self->folder);
      self->folder = g_strdup(g_value_get_string(value));
      break;
    case PROP_FILENAME:
      if (self->filename != NULL)
        g_free (self->filename);
      self->filename = g_strdup(g_value_get_string(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
entry_get_property (GObject    *object,
                    guint       property_id,
                    GValue     *value,
                    GParamSpec *pspec)
{
  Entry *self = DIARY_ENTRY (object);

  switch ((EntryProperties) property_id)
    {
    case PROP_FOLDER:
      g_value_set_string (value, self->folder);
      break;
    case PROP_FILENAME:
      g_value_set_string (value, self->filename);
      break;
    case PROP_BASENAME: {
      gchar *basename = filename_to_basename (self->filename);
      g_value_set_string (value, basename);
      g_free (basename);
      break;
    }
    case PROP_PATH: {
      gchar *path;
      path = g_strdup_printf ("%s/%s", self->folder, self->filename);
      g_value_set_string (value, path);
      g_free (path);
      break;
    }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

gchar *
entry_read(Entry *self, GError **err)
{
  gchar *filepath;
  gchar *body;
  gsize length;
  GError *err_local = NULL;

  g_object_get (self, "filepath", &filepath, NULL);
  if (!g_file_get_contents (filepath, &body, &length, &err_local)) {
    if (err_local->code == G_FILE_ERROR_NOENT) {
      body = g_strdup ("");
      g_clear_error (&err_local);
      goto cleanup;
    }
    g_error ("failed to read entry: %s", err_local->message);
  }

cleanup:
  if (err_local != NULL && err != NULL) {
    *err = err_local;
  }
  g_free (filepath);
  return body;
}

gboolean
entry_write (Entry *self, gchar *text, GError **err)
{
  gchar *filepath;
  gboolean ret;

  g_object_get (self, "filepath", &filepath, NULL);

  ret = g_file_set_contents (filepath, text, -1, err);

  g_print ("saved %s\n", filepath);

  g_free (filepath);

  return ret;
}

gboolean
entry_delete (Entry *self)
{
  gboolean ret = FALSE;
  GFile *file;
  GDir *dir;
  gchar *filepath = NULL;
  const gchar *filename = NULL;
  gchar *basename = NULL;
  gchar *photos_path = NULL;
  GError *err = NULL;

  g_object_get (self, "filepath", &filepath, NULL);
  file = g_file_new_for_path (filepath);

  g_print ("deleting diary entry %s\n", filepath);
  if (!g_file_delete (file, NULL, &err)) {
    g_warning ("Failed to remove file '%s': %s", filepath, err->message);
    g_object_unref (file);
    g_clear_error (&err);
    g_free (filepath);
    goto cleanup;
  }
  g_object_unref (file);

  basename = filename_to_basename (self->filename);

  photos_path = g_strdup_printf ("%s/photos/%s", self->folder, basename);
  g_print ("deleting photos in '%s'\n", photos_path);
  dir = g_dir_open(photos_path, 0, &err);
  while (dir && (filename = g_dir_read_name(dir))) {
    filepath = g_strdup_printf ("%s/%s", photos_path, filename);
    file = g_file_new_for_path (filepath);
    if (!g_file_delete (file, NULL, &err)) {
      g_warning ("Failed to remove file '%s': %s", filepath, err->message);
      g_object_unref (file);
      g_free (filepath);
      g_clear_error (&err);
      goto cleanup;
    }
    g_object_unref (file);
    g_free (filepath);
  }
  if (g_remove (photos_path) < 0 && errno != ENOENT) {
    g_printerr ("Failed to delete photos folder: %s\n", strerror (errno));
  }

  ret = TRUE;

cleanup:
  g_free (photos_path);

  return ret;
}

static void
entry_class_init (EntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = entry_get_property;
  object_class->set_property = entry_set_property;

  obj_properties[PROP_FOLDER] =
    g_param_spec_string ("folder",
                         "folder",
                         "folder",
                         NULL  /* default value */,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  obj_properties[PROP_FILENAME] =
    g_param_spec_string ("filename",
                         "filename",
                         "filename",
                         NULL  /* default value */,
                         G_PARAM_READWRITE);
  obj_properties[PROP_BASENAME] =
    g_param_spec_string ("basename",
                         "basename",
                         "basename",
                         NULL  /* default value */,
                         G_PARAM_READABLE);
  obj_properties[PROP_PATH] =
    g_param_spec_string ("filepath",
                         "filepath",
                         "Path of file where the diary entry is saved",
                         NULL,
                         G_PARAM_READABLE);

  g_object_class_install_properties (object_class,
                                     NUM_PROPS,
                                     obj_properties);
}

static void
entry_init (Entry *self)
{
  self->folder = NULL;
  self->filename = NULL;
}

Entry *
entry_open (const gchar *folder, const gchar *filename)
{
  return g_object_new (DIARY_TYPE_ENTRY,
                       "folder", folder,
                       "filename", filename,
                       NULL);
}

Entry *
entry_new (void)
{
    gchar *folder = utils_get_diary_folder ();
    GDateTime *now = g_date_time_new_now_local ();
    gchar *filename = g_date_time_format (now, "%Y-%m-%d - %A.md");
    Entry *entry = DIARY_ENTRY (entry_open (folder, filename));
    g_free (folder);
    return entry;
}
