#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DIARY_TYPE_ENTRY_DELETE_DIALOG (entry_delete_dialog_get_type())

G_DECLARE_FINAL_TYPE (EntryDeleteDialog, entry_delete_dialog, DIARY, ENTRY_DELETE_DIALOG, GtkWindow)

GtkWidget *entry_delete_dialog_new(void);

void entry_delete_dialog_open(EntryDeleteDialog *self, GtkWindow *parent);

// Signals:
// "confirmed" ()
// "cancelled" ()

G_END_DECLS
