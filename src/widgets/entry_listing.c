#include "entry_listing.h"
#include "entry_view.h"
#include "window.h"
#include "entry.h"

struct _EntryListing
{
  GtkBox parent_instance;

  GtkLabel *name_label;

  Entry *entry;
};

G_DEFINE_TYPE (EntryListing, entry_listing, GTK_TYPE_BOX);

static void
entry_listing_class_init (EntryListingClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/johan-bjareholt/simple-diary/ui/entry_listing.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryListing, name_label);
}

static void
entry_listing_init (EntryListing *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->entry = NULL;
}

Entry *
entry_listing_get_entry (EntryListing *listing)
{
  return listing->entry;
}

static void
entry_listing_update_title (EntryListing *self)
{
  gchar *title;

  g_object_get (self->entry, "basename", &title, NULL);
  gtk_label_set_label(self->name_label, title);
}

void
on_entry_filename_changed (Entry *entry, GParamSpecString *filename, gpointer user_data)
{
  EntryListing *listing = DIARY_ENTRY_LISTING (user_data);

  g_print ("test\n");

  entry_listing_update_title (listing);
}

GtkWidget *
entry_listing_new (Entry *entry)
{
  EntryListing *listing = g_object_new (DIARY_TYPE_ENTRY_LISTING, NULL);
  listing->entry = entry;

  entry_listing_update_title (listing);

  g_signal_connect (entry, "notify::filename", G_CALLBACK (on_entry_filename_changed), listing);

  return GTK_WIDGET (listing);
}
