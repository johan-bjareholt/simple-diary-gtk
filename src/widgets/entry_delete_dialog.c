#include <gtk/gtk.h>

#include "widgets/entry_delete_dialog.h"

struct _EntryDeleteDialog
{
  GtkDialog parent_instance;

  /* widgets */
  GtkButton *button_cancel;
  GtkButton *button_ok;
};

struct _EntryDeleteDialogClass
{
  GtkDialogClass parent_class;
};

G_DEFINE_TYPE (EntryDeleteDialog, entry_delete_dialog, GTK_TYPE_DIALOG);

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
