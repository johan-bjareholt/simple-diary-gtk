#include <webkit2/webkit2.h>

#include "entry_view.h"
#include "window.h"
#include "entry.h"
#include "entry_edit.h"
#include "md2html.h"

struct _EntryView
{
  GtkBox parent_instance;

  GtkButton *edit_button;
  GtkButton *delete_button;
  GtkWidget *webview;

  Entry *entry;
};

G_DEFINE_TYPE (EntryView, entry_view, GTK_TYPE_BOX);

static void
entry_view_class_init (EntryViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/johan-bjareholt/simple-diary/ui/entry_view.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryView, edit_button);
  gtk_widget_class_bind_template_child (widget_class, EntryView, delete_button);
}

static gboolean
edit_button_pressed (GtkButton *button, gpointer user_data)
{
  GtkWidget *entry_edit;
  DiaryWindow *diary_window;
  EntryView *self = (EntryView *) user_data;

  diary_window = diary_window_get_instance ();

  entry_edit = entry_edit_new (self->entry);
  diary_window_push_view (diary_window, entry_edit);

  return FALSE;
}

static void
delete (GtkDialog *dialog, int response, gpointer user_data)
{
  DiaryWindow *diary_window;
  EntryView *self = (EntryView *) user_data;

  diary_window = diary_window_get_instance ();

  switch (response) {
    case GTK_RESPONSE_YES:
      entry_delete (self->entry, NULL);
      diary_window_pop_view (diary_window);
      break;
    case GTK_RESPONSE_NO:
      break;
    default:
      g_assert_not_reached ();
      break;
  }
}

static gboolean
delete_button_pressed (GtkButton *button, gpointer user_data)
{
  DiaryWindow *diary_window;
  EntryView *self = (EntryView *) user_data;
  GtkWidget *dialog, *label, *content_area;
  GtkDialogFlags flags;

  diary_window = diary_window_get_instance ();

  flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  dialog = gtk_dialog_new_with_buttons (
      "Confirm deletion of entry",
      GTK_WINDOW (diary_window),
      flags,
      "Yes",
      GTK_RESPONSE_YES,
      "No",
      GTK_RESPONSE_NO,
      NULL);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  label = gtk_label_new ("Are you sure you want to delete this entry?");

  g_signal_connect (dialog,
                    "response",
                    G_CALLBACK (delete),
                    self);

 gtk_container_add (GTK_CONTAINER (content_area), label);
 gtk_widget_show_all (dialog);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  return TRUE;
}

static void
entry_view_init (EntryView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->entry = NULL;
  self->webview = webkit_web_view_new ();

  g_signal_connect (self->edit_button, "clicked", (GCallback) edit_button_pressed, self);
  g_signal_connect (self->delete_button, "clicked", (GCallback) delete_button_pressed, self);
}

GtkWidget *
entry_view_new (Entry *entry)
{
  GError *err = NULL;
  EntryView *self;

  self = g_object_new (DIARY_TYPE_ENTRY_VIEW, NULL);
  self->entry = entry;

  gchar *text_md = entry_read (entry, &err);
  gchar *text_html = md2html (text_md, &err);

  // TODO: move to constructed cb
  gtk_box_pack_start (GTK_BOX (self), self->webview, TRUE, TRUE, 0);
  gchar *text_html_full = g_strdup_printf ("<html>\n<body>\n%s\n</body>\n</html>", text_html);
  // TODO: Set relative folder (3rd arg)
  webkit_web_view_load_html (WEBKIT_WEB_VIEW (self->webview), text_html_full, NULL);

  gtk_widget_show_all (GTK_WIDGET (self));

  return GTK_WIDGET (self);
}
