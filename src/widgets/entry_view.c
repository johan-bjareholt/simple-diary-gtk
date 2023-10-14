#include <gtkmdview.h>

#include "entry_view.h"
#include "window.h"
#include "entry.h"
#include "entry_edit.h"
#include "entry_rename_dialog.h"
#include "entry_delete_dialog.h"
#include "settings.h"

struct _EntryView
{
  GtkBox parent_instance;

  GtkButton *edit_button;
  GtkButton *rename_button;
  GtkButton *delete_button;

  GtkWidget *md_view;
  GtkWidget *md_viewport;

  Entry *entry;
};

enum
{
  SIGNAL_DELETED,
  LAST_SIGNAL,
};

static guint entry_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EntryView, entry_view, GTK_TYPE_BOX);

static void
entry_view_class_init (EntryViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/entry_view.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryView, md_viewport);
  gtk_widget_class_bind_template_child (widget_class, EntryView, edit_button);
  gtk_widget_class_bind_template_child (widget_class, EntryView, rename_button);
  gtk_widget_class_bind_template_child (widget_class, EntryView, delete_button);

  entry_view_signals [SIGNAL_DELETED] =
      g_signal_new ("deleted", G_TYPE_FROM_CLASS (klass),
      0, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, DIARY_TYPE_ENTRY);
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
rename_finished (GtkDialog *dialog, int response_id, gpointer user_data)
{
  EntryView *entry_view = DIARY_ENTRY_VIEW (user_data);
  EntryRenameDialog *rename_dialog = DIARY_ENTRY_RENAME_DIALOG (dialog);

  if (response_id == GTK_RESPONSE_OK) {
    gchar *text = entry_rename_dialog_get_name (rename_dialog);
    // TODO: check if name contains dots or slashes
    gchar *new_name = g_strdup_printf ("%s.md", text);
    g_print ("Renaming entry to '%s'\n", new_name);
    entry_rename_file (entry_view->entry, new_name);
    g_free (text);
    g_free (new_name);
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static gboolean
rename_button_clicked (GtkButton *button, gpointer user_data)
{
  EntryView *self = (EntryView *) user_data;
  GtkWindow *window;
  GtkWidget *dialog;
  gchar *basename;

  window = GTK_WINDOW (diary_window_get_instance ());
  g_object_get (self->entry, "basename", &basename, NULL);

  dialog = entry_rename_dialog_new (basename);

  g_signal_connect (dialog,
                    "response",
                    G_CALLBACK (rename_finished),
                    self);

  gtk_window_set_transient_for (GTK_WINDOW (dialog), window);

  gtk_widget_set_visible (dialog, TRUE);

  g_free (basename);

  return TRUE;
}

static void
delete_finished (GtkDialog *dialog, int response_id, gpointer user_data)
{
  EntryView *entry_view = DIARY_ENTRY_VIEW (user_data);
  Entry *entry = entry_view->entry;

  switch (response_id) {
    case GTK_RESPONSE_YES:
      /* At this point, entry_view will get deleted, so we need to save it to a
       * local variable and ref it before emitting deleted */
      g_object_ref (entry);
      g_signal_emit_by_name (entry_view, "deleted", entry_view->entry);
      entry_delete (entry);
      g_object_unref (entry);
      break;
    case GTK_RESPONSE_NO:
    case GTK_RESPONSE_DELETE_EVENT:
      break;
    default:
      g_assert_not_reached ();
      break;
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

static gboolean
delete_button_clicked (GtkButton *button, gpointer user_data)
{
  EntryView *self = (EntryView *) user_data;
  DiaryWindow *window;
  GtkWidget *dialog;

  window = diary_window_get_instance ();

  dialog = entry_delete_dialog_new ();

  g_signal_connect (dialog,
                    "response",
                    G_CALLBACK (delete_finished),
                    self);

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));

  gtk_widget_set_visible (dialog, TRUE);

  return TRUE;
}

static void
entry_view_load_md (EntryView *self)
{
  GError *err = NULL;
  gchar *text_md;
  gchar *folder;

  g_object_get (self->entry, "folder", &folder, NULL);

  text_md = entry_read (self->entry, &err);

  self->md_view = gtk_md_view_new (text_md, folder);

  gtk_viewport_set_child (GTK_VIEWPORT (self->md_viewport), self->md_view);

  //g_free (folder_uri);
  g_free (text_md);
  g_free (folder);
}

static void
entry_view_init (EntryView *self)
{
  g_type_ensure (GTK_TYPE_MD_VIEW);

  gtk_widget_init_template (GTK_WIDGET (self));

  self->entry = NULL;

  g_signal_connect (self->edit_button, "clicked", (GCallback) edit_button_clicked, self);
  g_signal_connect (self->rename_button, "clicked", (GCallback) rename_button_clicked, self);
  g_signal_connect (self->delete_button, "clicked", (GCallback) delete_button_clicked, self);

  // This is necessary so it's reloaded if you edit an entry and go back
  g_assert (g_signal_connect (self, "map", G_CALLBACK (entry_view_load_md), NULL) > 0);
}

GtkWidget *
entry_view_new (Entry *entry)
{
  EntryView *self;

  self = g_object_new (DIARY_TYPE_ENTRY_VIEW, NULL);
  self->entry = entry;

  return GTK_WIDGET (self);
}
