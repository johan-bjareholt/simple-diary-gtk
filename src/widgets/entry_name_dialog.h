#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DIARY_TYPE_ENTRY_NAME_DIALOG (entry_name_dialog_get_type())

G_DECLARE_FINAL_TYPE (EntryNameDialog, entry_name_dialog, DIARY, ENTRY_NAME_DIALOG, GtkWindow)

GtkWidget *entry_name_dialog_new(void);

void entry_name_dialog_open(EntryNameDialog *self, GtkWindow *parent);

// Signals:
// "confirmed" (gchar *name)
// "cancelled" ()

G_END_DECLS
