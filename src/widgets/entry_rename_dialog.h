#pragma once

#include <gtk/gtk.h>

#include "entry.h"

G_BEGIN_DECLS

#define DIARY_TYPE_ENTRY_RENAME_DIALOG (entry_rename_dialog_get_type())

#define DIARY_TYPE_IS_ENTRY_RENAME_DIALOG (object) \
    (G_OBJECT_TYPE (object) == DIARY_TYPE_ENTRY_RENAME_DIALOG)

G_DECLARE_FINAL_TYPE (EntryRenameDialog, entry_rename_dialog, DIARY, ENTRY_RENAME_DIALOG, GtkDialog)

GtkWidget *
entry_rename_dialog_new (const gchar * basename);

gchar *
entry_rename_dialog_get_name (EntryRenameDialog * rename_dialog);

G_END_DECLS
