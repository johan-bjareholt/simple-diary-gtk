#pragma once

#include <gtk/gtk.h>

#include "entry.h"

G_BEGIN_DECLS

#define DIARY_TYPE_ENTRY_DELETE_DIALOG (entry_delete_dialog_get_type())

#define DIARY_TYPE_IS_ENTRY_DELETE_DIALOG (object) \
    (G_OBJECT_TYPE (object) == DIARY_TYPE_ENTRY_DELETE_DIALOG)

G_DECLARE_FINAL_TYPE (EntryDeleteDialog, entry_delete_dialog, DIARY, ENTRY_DELETE_DIALOG, GtkWindow)

GtkWidget *
entry_delete_dialog_new ();

void
entry_delete_dialog_open (EntryDeleteDialog *self,
                          GtkWindow *parent,
                          GAsyncReadyCallback callback,
                          gpointer user_data);

gboolean
entry_delete_dialog_open_finish (EntryDeleteDialog *self,
                                 GAsyncResult *result);

G_END_DECLS
