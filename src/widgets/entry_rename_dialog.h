#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DIARY_TYPE_ENTRY_RENAME_DIALOG (entry_rename_dialog_get_type())

G_DECLARE_FINAL_TYPE (EntryRenameDialog, entry_rename_dialog, DIARY, ENTRY_RENAME_DIALOG, GtkWindow)

GtkWidget *entry_rename_dialog_new(const gchar *basename);

void entry_rename_dialog_open(EntryRenameDialog *self, GtkWindow *parent);

// Signals:
// "confirmed" (gchar *name)
// "cancelled" ()

G_END_DECLS
