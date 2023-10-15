#include <gtk/gtk.h>

#include "widgets/entry_name_dialog.h"

struct _EntryNameDialog
{
  GtkWindow parent_instance;

  /* widgets */
  GtkEntry  *name_entry;
  GtkButton *button_cancel;
  GtkButton *button_ok;
  GtkButton *date_button;
  GtkLabel *date_label;
  GtkPopover *date_popover;
  GtkCalendar *date_calendar;
};

struct _EntryNameDialogClass
{
  GtkWindowClass parent_class;
};

G_DEFINE_TYPE (EntryNameDialog, entry_name_dialog, GTK_TYPE_WINDOW);

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
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      date_button);
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      date_label);
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      date_popover);
  gtk_widget_class_bind_template_child (widget_class, EntryNameDialog,
      date_calendar);
}

static void
date_button_clicked (GtkEntry *entry, gpointer user_data)
{
  EntryNameDialog *dialog = DIARY_ENTRY_NAME_DIALOG (user_data);
  gtk_widget_set_visible (GTK_WIDGET (dialog->date_popover), TRUE);
}

static void
entry_name_dialog_update_date_label (EntryNameDialog *self)
{
  gint day, month, year;
  gchar *date_str;

  g_object_get (self->date_calendar, "day", &day, "month", &month,
          "year", &year, NULL);
  date_str = g_strdup_printf ("%d-%02d-%02d", year, month+1, day);
  g_object_set (self->date_label, "label", date_str, NULL);

  g_free (date_str);
}

static void
entry_name_dialog_init (EntryNameDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  entry_name_dialog_update_date_label (self);
  g_signal_connect (self->date_button, "clicked",
      G_CALLBACK (date_button_clicked), self);
  g_signal_connect_swapped (self->date_calendar, "day-selected",
      G_CALLBACK (entry_name_dialog_update_date_label), self);
}

GtkWidget *
entry_name_dialog_new (void)
{
    EntryNameDialog *entry_name_dialog;
    GtkEntryBuffer *buffer;

    entry_name_dialog = g_object_new (DIARY_TYPE_ENTRY_NAME_DIALOG, NULL);
    buffer = gtk_entry_buffer_new ("Title", -1);
    gtk_entry_set_buffer (entry_name_dialog->name_entry, buffer);

    return GTK_WIDGET (entry_name_dialog);
}

static void
dialog_response (GtkEntry *entry,
                 gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    EntryNameDialog *self = DIARY_ENTRY_NAME_DIALOG (g_task_get_task_data (task));
    gchar *text = entry_name_dialog_get_name (self);
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
entry_name_dialog_open (EntryNameDialog *self,
                        GtkWindow *parent,
                        GAsyncReadyCallback callback,
                        gpointer user_data)
{
  GTask *task;

  task = g_task_new (self, FALSE, callback, user_data);
  g_task_set_check_cancellable (task, FALSE);
  g_task_set_source_tag (task, entry_name_dialog_open);
  g_task_set_task_data (task, self, g_object_unref);

  g_signal_connect (self->name_entry, "activate", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->button_ok, "clicked", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->button_cancel, "clicked", G_CALLBACK (dialog_cancel), task);

  gtk_window_set_transient_for (GTK_WINDOW (self), parent);
  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
}

gchar *
entry_name_dialog_open_finish (EntryNameDialog *self,
                               GAsyncResult *result)
{
  gtk_window_destroy (GTK_WINDOW (self));

  return g_task_propagate_pointer (G_TASK (result), NULL);
}

gchar *
entry_name_dialog_get_name (EntryNameDialog * name_dialog)
{
    GtkEntryBuffer *buffer;
    const gchar *title;
    gchar *date;
    gchar *name;

    buffer = gtk_entry_get_buffer (name_dialog->name_entry);
    title = gtk_entry_buffer_get_text (buffer);

    g_object_get (name_dialog->date_label, "label", &date, NULL);

    name = g_strdup_printf ("%s - %s", date, title);

    g_free (date);

    return name;
}
