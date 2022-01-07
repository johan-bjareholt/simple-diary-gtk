#include <gtk/gtk.h>

#include "widgets/entry_name_dialog.h"

struct _EntryNameDialog
{
  GtkDialog parent_instance;

  /* widgets */
  GtkEntry  *name_entry;
  GtkButton *button_cancel;
  GtkButton *button_ok;
};

struct _EntryNameDialogClass
{
  GtkDialogClass parent_class;
};

G_DEFINE_TYPE (EntryNameDialog, entry_name_dialog, GTK_TYPE_DIALOG);

static void
entry_name_dialog_class_init (EntryNameDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/entry_name_dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      name_entry);
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      button_cancel);
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      button_ok);
}

static void
dialog_text_box_activate (GtkEntry *entry, gpointer user_data)
{
  GtkDialog *dialog = GTK_DIALOG (user_data);
  gtk_dialog_response (dialog, GTK_RESPONSE_OK);
}

static void
entry_name_dialog_init (EntryNameDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->name_entry, "activate", (GCallback) dialog_text_box_activate, self);
}

GtkWidget *
entry_name_dialog_new (const gchar * basename)
{
    EntryNameDialog *entry_name_dialog;
    GtkEntryBuffer *buffer;

    entry_name_dialog = g_object_new (DIARY_TYPE_ENTRY_NAME_DIALOG, NULL);
    buffer = gtk_entry_buffer_new (basename, -1);
    gtk_entry_set_buffer (entry_name_dialog->name_entry, buffer);

    return GTK_WIDGET (entry_name_dialog);
}

gchar *
entry_name_dialog_get_name (EntryNameDialog * name_dialog)
{
    GtkEntryBuffer *buffer;
    const gchar *text;

    buffer = gtk_entry_get_buffer (name_dialog->name_entry);
    text = gtk_entry_buffer_get_text (buffer);

    return g_strdup (text);
}
