#include <gtk/gtk.h>

#include "utils.h"
#include "widgets/entry_rename_dialog.h"

struct _EntryRenameDialog
{
  GtkWindow parent_instance;

  /* widgets */
  GtkEntry  *name_entry;
  GtkButton *button_cancel;
  GtkButton *button_ok;
};

struct _EntryRenameDialogClass
{
  GtkWindowClass parent_class;
};

G_DEFINE_TYPE (EntryRenameDialog, entry_rename_dialog, GTK_TYPE_WINDOW);

static void
entry_rename_dialog_class_init (EntryRenameDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/entry_rename_dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryRenameDialog,
      name_entry);
  gtk_widget_class_bind_template_child (widget_class, EntryRenameDialog,
      button_cancel);
  gtk_widget_class_bind_template_child (widget_class, EntryRenameDialog,
      button_ok);
}

static void
entry_rename_dialog_init (EntryRenameDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
entry_rename_dialog_new (const gchar * basename)
{
    EntryRenameDialog *entry_rename_dialog;
    GtkEntryBuffer *buffer;

    entry_rename_dialog = g_object_new (DIARY_TYPE_ENTRY_RENAME_DIALOG, NULL);
    buffer = gtk_entry_buffer_new (basename, -1);
    gtk_entry_set_buffer (entry_rename_dialog->name_entry, buffer);

    return GTK_WIDGET (entry_rename_dialog);
}

static void
dialog_response (GtkEntry *entry,
                 gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    EntryRenameDialog *self = DIARY_ENTRY_RENAME_DIALOG (g_task_get_task_data (task));
    gchar *text = entry_rename_dialog_get_name (self);
    if (strchr(text, '.') != NULL || strchr(text, '/') != NULL) {
        utils_error_dialog(GTK_WIDGET(entry), "Slashes and dots are not allowed in titles");
        return;
    }
    g_task_return_pointer (task, text, g_free);
}

static void
dialog_cancel (GtkEntry *entry,
                 gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    g_task_return_pointer (task, NULL, g_free);
}

void
entry_rename_dialog_open (EntryRenameDialog *self,
                          GtkWindow *parent,
                          GAsyncReadyCallback callback,
                          gpointer user_data)
{
  GTask *task;

  task = g_task_new (self, FALSE, callback, user_data);
  g_task_set_check_cancellable (task, FALSE);
  g_task_set_source_tag (task, entry_rename_dialog_open);
  g_task_set_task_data (task, self, g_object_unref);

  g_signal_connect (self->name_entry, "activate", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->button_ok, "clicked", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->button_cancel, "clicked", G_CALLBACK (dialog_cancel), task);

  gtk_window_set_transient_for (GTK_WINDOW (self), parent);
  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
}

gchar *
entry_rename_dialog_open_finish (EntryRenameDialog *self,
                                 GAsyncResult *result)
{
  gtk_window_destroy (GTK_WINDOW (self));

  return g_task_propagate_pointer (G_TASK (result), NULL);
}

gchar *
entry_rename_dialog_get_name (EntryRenameDialog * rename_dialog)
{
    GtkEntryBuffer *buffer;
    const gchar *text;

    buffer = gtk_entry_get_buffer (rename_dialog->name_entry);
    text = gtk_entry_buffer_get_text (buffer);

    return g_strdup (text);
}
