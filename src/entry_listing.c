#include "entry_listing.h"
#include "entry_view.h"
#include "window.h"
#include "entry.h"

struct _EntryListing
{
  GtkBox parent_instance;

  GtkLabel *name_label;
  GtkButton *edit_button;

  Entry *entry;
};

G_DEFINE_TYPE (EntryListing, entry_listing, GTK_TYPE_BOX);

static void
entry_listing_class_init (EntryListingClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/johan-bjareholt/simple-diary/ui/entry_listing.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryListing, name_label);
  gtk_widget_class_bind_template_child (widget_class, EntryListing, edit_button);
}
#include <webkit2/webkit2.h>

static gboolean
edit_button_pressed (GtkButton *button, gpointer user_data)
{
  DiaryWindow *diary_window;
  EntryListing *self = (EntryListing *) user_data;
  GtkWidget *entry_view;

  diary_window = diary_window_get_instance ();

  entry_view = entry_view_new (self->entry);
  diary_window_push_view (diary_window, entry_view);

  return FALSE;
}

static void
entry_listing_init (EntryListing *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->entry = NULL;

  g_signal_connect (self->edit_button, "clicked", (GCallback) edit_button_pressed, self);
}

GtkWidget *
entry_listing_new (Entry *entry)
{
  EntryListing *listing = g_object_new (DIARY_TYPE_ENTRY_LISTING, NULL);
  listing->entry = entry;

  gchar *title;
  g_object_get (entry, "filename", &title, NULL);
  gtk_label_set_label(listing->name_label, title);

  return GTK_WIDGET (listing);
}
