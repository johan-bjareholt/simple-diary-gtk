#include <gtk/gtk.h>

#include "widgets/entry_rename_dialog.h"

struct _EntryRenameDialog
{
  GtkDialog parent_instance;

  /* widgets */
  GtkEntry  *name_entry;
  GtkButton *button_cancel;
  GtkButton *button_ok;
};

struct _EntryRenameDialogClass
{
  GtkDialogClass parent_class;
};

G_DEFINE_TYPE (EntryRenameDialog, entry_rename_dialog, GTK_TYPE_DIALOG);

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
dialog_text_box_activate (GtkEntry *entry, gpointer user_data)
{
  GtkDialog *dialog = GTK_DIALOG (user_data);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  gtk_dialog_response (dialog, GTK_RESPONSE_OK);
G_GNUC_END_IGNORE_DEPRECATIONS
}

static void
entry_rename_dialog_init (EntryRenameDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->name_entry, "activate", (GCallback) dialog_text_box_activate, self);
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

gchar *
entry_rename_dialog_get_name (EntryRenameDialog * rename_dialog)
{
    GtkEntryBuffer *buffer;
    const gchar *text;

    buffer = gtk_entry_get_buffer (rename_dialog->name_entry);
    text = gtk_entry_buffer_get_text (buffer);

    return g_strdup (text);
}
