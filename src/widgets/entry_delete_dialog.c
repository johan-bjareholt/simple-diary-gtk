#include <gtk/gtk.h>

#include "widgets/entry_delete_dialog.h"

struct _EntryDeleteDialog
{
  GtkWindow parent_instance;

  /* widgets */
  GtkButton *button_cancel;
  GtkButton *button_ok;
};

struct _EntryDeleteDialogClass
{
  GtkWindowClass parent_class;
};

G_DEFINE_TYPE (EntryDeleteDialog, entry_delete_dialog, GTK_TYPE_WINDOW);

static void
entry_delete_dialog_class_init (EntryDeleteDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/entry_delete_dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryDeleteDialog,
      button_cancel);
  gtk_widget_class_bind_template_child (widget_class, EntryDeleteDialog,
      button_ok);
}

static void
entry_delete_dialog_init (EntryDeleteDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
entry_delete_dialog_new ()
{
    EntryDeleteDialog *entry_delete_dialog;

    entry_delete_dialog = g_object_new (DIARY_TYPE_ENTRY_DELETE_DIALOG, NULL);

    return GTK_WIDGET (entry_delete_dialog);
}

static void
dialog_response (GtkEntry *entry,
                 gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    g_task_return_boolean (task, TRUE);
}

static void
dialog_cancel (GtkEntry *entry,
               gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    g_task_return_boolean (task, FALSE);
}

void
entry_delete_dialog_open (EntryDeleteDialog *self,
                          GtkWindow *parent,
                          GAsyncReadyCallback callback,
                          gpointer user_data)
{
  GTask *task;

  task = g_task_new (self, FALSE, callback, user_data);
  g_task_set_check_cancellable (task, FALSE);
  g_task_set_source_tag (task, entry_delete_dialog_open);
  g_task_set_task_data (task, self, g_object_unref);

  g_signal_connect (self->button_ok, "clicked", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->button_cancel, "clicked", G_CALLBACK (dialog_cancel), task);

  gtk_window_set_transient_for (GTK_WINDOW (self), parent);
  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
}

gboolean
entry_delete_dialog_open_finish (EntryDeleteDialog *self,
                                 GAsyncResult *result)
{
  gtk_window_destroy (GTK_WINDOW (self));

  return g_task_propagate_boolean (G_TASK (result), NULL);
}
