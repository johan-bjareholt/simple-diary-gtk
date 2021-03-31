#include <webkit2/webkit2.h>

#include "entry_view.h"
#include "window.h"
#include "entry.h"
#include "entry_edit.h"
#include "md2html.h"
#include "settings.h"

struct _EntryView
{
  GtkBox parent_instance;

  GtkButton *edit_button;
  GtkButton *rename_button;
  GtkButton *delete_button;

  GtkWidget *webview;

  Entry *entry;
};

G_DEFINE_TYPE (EntryView, entry_view, GTK_TYPE_BOX);

static void
entry_view_class_init (EntryViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/johan-bjareholt/simple-diary/ui/entry_view.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryView, webview);
  gtk_widget_class_bind_template_child (widget_class, EntryView, edit_button);
  gtk_widget_class_bind_template_child (widget_class, EntryView, rename_button);
  gtk_widget_class_bind_template_child (widget_class, EntryView, delete_button);
}

static gboolean
edit_button_clicked (GtkButton *button, gpointer user_data)
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
dialog_text_box_activate (GtkEntry *entry, gpointer user_data)
{
  GtkDialog *dialog = GTK_DIALOG (user_data);
  gtk_dialog_response (dialog, GTK_RESPONSE_OK);
}

static gboolean
rename_button_clicked (GtkButton *button, gpointer user_data)
{
  EntryView *self = (EntryView *) user_data;
  GtkWindow *window = GTK_WINDOW (diary_window_get_instance ());
  GtkWidget *dialog =
    gtk_message_dialog_new (window,
                            GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
                            GTK_MESSAGE_QUESTION,
                            GTK_BUTTONS_OK_CANCEL,
                            "What do you want to rename it to?");
  GtkWidget *dialog_box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gchar *basename;
  g_object_get (self->entry, "basename", &basename, NULL);
  GtkWidget *text_input = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (text_input), basename);
  gtk_container_add (GTK_CONTAINER (dialog_box), text_input);
  g_signal_connect (text_input, "activate", (GCallback) dialog_text_box_activate, dialog);

  gtk_widget_show_all (dialog);

  gint ret = gtk_dialog_run (GTK_DIALOG (dialog));
  if (ret == GTK_RESPONSE_OK) {
    const gchar *text = gtk_entry_get_text (GTK_ENTRY (text_input));
    /* TODO: check if name contains dots or slashes */
    g_print ("%s\n", text);
    gchar *new_name = g_strdup_printf ("%s.md", text);
    entry_rename_file (self->entry, new_name);
  }

  gtk_widget_destroy (dialog);

  return FALSE;
}

static gboolean
delete_button_clicked (GtkButton *button, gpointer user_data)
{
  DiaryWindow *diary_window;
  EntryView *self = (EntryView *) user_data;
  GtkWidget *dialog;
  gint response;

  diary_window = diary_window_get_instance ();

  dialog =
    gtk_message_dialog_new (GTK_WINDOW (diary_window),
                            GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
                            GTK_MESSAGE_QUESTION,
                            GTK_BUTTONS_YES_NO,
                            "Are you sure you want to delete this entry?");

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  switch (response) {
    case GTK_RESPONSE_YES:
      gtk_widget_destroy (GTK_WIDGET (self));
      entry_delete (self->entry);
      break;
    case GTK_RESPONSE_NO:
    case GTK_RESPONSE_DELETE_EVENT:
      break;
    default:
      g_assert_not_reached ();
      break;
  }

  gtk_widget_destroy (dialog);

  return TRUE;
}

static gchar *css_classes = "\
img {\n\
  max-width: 100%;\n\
  object-fit: contain;\n\
  overflow: hidden;\n\
}\n\
table, th, td {\n\
  border: 0.02em solid black;\n\
  border-collapse: collapse;\n\
  padding: 0.2em;\n\
}\n\
body {\n\
  width: 100%;\n\
  padding: 0;\n\
  border: 0;\n\
  margin: 0;\n\
}\n\
\n\
body.light {}\n\
body.dark {\n\
  color: #fff;\n\
  background-color: #333;\n\
}\n\
";

static gchar *html_full_format = "\
<html>\n\
<head>\n\
<style>\n\
%s\n\
</style>\n\
</head>\n\
<body class=\"%s\">\n\
%s\n\
</body>\n\
</html>\n\
";

static void
entry_view_load_html (EntryView *self)
{
  GError *err = NULL;
  gchar *text_md, *text_html, *text_html_full;
  gchar *folder;
  gboolean dark_mode;
  gchar *body_class;

  g_object_get (self->entry, "folder", &folder, NULL);
  dark_mode = settings_get_dark_mode ();
  body_class = dark_mode ? "dark" : "light";

  text_md = entry_read (self->entry, &err);
  text_html = md2html (text_md, &err);
  text_html_full = g_strdup_printf (html_full_format, css_classes, body_class, text_html);
  //g_print (text_html_full);
  gchar *folder_uri = g_strdup_printf ("file://%s/", folder);
  webkit_web_view_load_html (WEBKIT_WEB_VIEW (self->webview), text_html_full, folder_uri);

  gtk_widget_show_all (GTK_WIDGET (self));

  g_free (folder_uri);
  g_free (text_md);
  g_free (text_html);
  g_free (text_html_full);
  g_free (folder);
}

static void
entry_view_init (EntryView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->entry = NULL;

  g_signal_connect (self->edit_button, "clicked", (GCallback) edit_button_clicked, self);
  g_signal_connect (self->rename_button, "clicked", (GCallback) rename_button_clicked, self);
  g_signal_connect (self->delete_button, "clicked", (GCallback) delete_button_clicked, self);

  // This is necessary so it's reloaded if you edit an entry and go back
  g_assert (g_signal_connect (self, "map", G_CALLBACK (entry_view_load_html), NULL) > 0);
}

GtkWidget *
entry_view_new (Entry *entry)
{
  EntryView *self;

  self = g_object_new (DIARY_TYPE_ENTRY_VIEW, NULL);
  self->entry = entry;

  return GTK_WIDGET (self);
}
