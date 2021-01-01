#include <gio/gio.h>

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
      gchar *basename;
      gchar *last_dot;
      basename = g_path_get_basename (self->filename);
      last_dot = strrchr (basename, '.');
      last_dot[0] = '\0';
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
entry_delete (Entry *self, GError **err)
{
  gboolean ret = FALSE;
  GFile *file;
  gchar *filepath;

  g_object_get (self, "filepath", &filepath, NULL);
  file = g_file_new_for_path (filepath);

  g_print ("deleting %s\n", filepath);
  if (!g_file_delete (file, NULL, err)) {
    if (err != NULL) {
      g_warning ("Failed to remove file '%s': %s", filepath, (*err)->message);
    }
    goto cleanup;
  }

  ret = TRUE;

cleanup:
  g_object_unref (file);
  g_free (filepath);

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
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
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
