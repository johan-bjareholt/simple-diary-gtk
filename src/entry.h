#pragma once

#include <glib-object.h>

#define DIARY_TYPE_ENTRY (entry_get_type())

G_DECLARE_FINAL_TYPE (Entry, entry, DIARY, ENTRY, GObject)

typedef struct _Entry Entry;

Entry * entry_new (void);
Entry * entry_open (const gchar *folder, const gchar *filename);
gchar * entry_read (Entry *self, GError **err);
gboolean entry_rename_file (Entry *self, gchar *new_name);
gboolean entry_write (Entry *self, gchar *text, GError **err);
gboolean entry_delete (Entry *self);
