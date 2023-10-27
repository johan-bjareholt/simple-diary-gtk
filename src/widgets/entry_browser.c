#include <adwaita.h>

#include "headerbuttons.h"
#include "entry_browser.h"
#include "entry_list.h"
#include "entry_listing.h"
#include "entry_edit.h"
#include "entry_view.h"
#include "entry_name_dialog.h"
#include "entry.h"
#include "window.h"
#include "utils.h"

struct _EntryBrowser
{
  GtkBox parent_instance;

  AdwLeaflet *leaflet;
  EntryList *entry_list;
  GtkBox *entry_list_box;
  GtkBox *content_box;
  GtkWidget *content_box_child;

  /* from header_buttons_control */
  GtkButton *back_button;
  GtkButton *new_button;
};

static void
update_header_buttons (EntryBrowser *self)
{
  if (self->content_box_child == NULL) {
    g_object_set (self->back_button, "visible", FALSE, NULL);
    g_object_set (self->new_button, "visible", TRUE, NULL);
  } else {
    g_object_set (self->back_button, "visible", TRUE, NULL);
    g_object_set (self->new_button, "visible", FALSE, NULL);
  }
}

static void
entry_browser_set_content (EntryBrowser *self, GtkWidget *widget)
{
  // Remove old content
  if (self->content_box_child != NULL) {
    gtk_box_remove (self->content_box, self->content_box_child);
  }

  // Add new content if there is any
  if (widget == NULL) {
    self->content_box_child = NULL;
    g_object_set (self->content_box, "visible", FALSE, NULL);
    gtk_widget_set_hexpand (GTK_WIDGET (self->entry_list_box), TRUE);
  } else {
    self->content_box_child = widget;
    gtk_box_append (self->content_box, self->content_box_child);
    g_object_set (self->content_box, "visible", TRUE, NULL);
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    adw_leaflet_set_visible_child (self->leaflet, GTK_WIDGET (self->content_box));
G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_widget_set_hexpand (GTK_WIDGET (self->entry_list_box), FALSE);
  }

  update_header_buttons (self);
}

static void
on_enter (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button)
{
  EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);

  self->new_button = new_button;
  self->back_button = back_button;

  g_object_set (settings_button, "visible", TRUE, NULL);
  update_header_buttons (self);
}

static void
on_leave (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button)
{
    EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);
    g_print ("leave %p\n", self);
}

static void
entry_view_deleted_cb (EntryView *entry_view, Entry *entry, gpointer user_data)
{
  gboolean removed;
  EntryBrowser *browser = DIARY_ENTRY_BROWSER (user_data);
  entry_browser_set_content (browser, NULL);
  removed = entry_list_remove (browser->entry_list, entry);
  g_assert (removed);
}

void
entry_name_cb (GObject *obj,
               GAsyncResult *res,
               gpointer user_data)
{
    EntryNameDialog *dialog = DIARY_ENTRY_NAME_DIALOG (obj);
    EntryBrowser *self = DIARY_ENTRY_BROWSER (user_data);

    g_autofree gchar *name = entry_name_dialog_open_finish(dialog, res);
    if (name != NULL) {
        gchar *text;
        gchar *filename;
        DiaryWindow *diary_window;
        GtkListBoxRow *entry_listing_row;
        EntryListing *entry_listing;
        GtkWidget *entry_view;
        GtkWidget *entry_edit;
        Entry *entry;

        diary_window = diary_window_get_instance ();

        text = entry_name_dialog_get_name (dialog);
        // TODO: check if name contains dots or slashes
        filename = g_strdup_printf ("%s.md", text);

        entry_listing_row = entry_list_find (self->entry_list, filename);
        if (entry_listing_row != NULL) {
          g_free (filename);
          entry_listing = DIARY_ENTRY_LISTING (gtk_list_box_row_get_child (entry_listing_row));
          entry = entry_listing_get_entry (entry_listing);
        } else {
          gchar *folder = utils_get_diary_folder ();
          entry = entry_new (folder, filename);
          g_free (folder);
          entry_list_add_entry (self->entry_list, entry);
          entry_listing_row = entry_list_find (self->entry_list, filename);
        }

        entry_edit = entry_edit_new (entry);
        entry_view = entry_view_new (entry);
        g_signal_connect (entry_view, "deleted", G_CALLBACK (entry_view_deleted_cb), self);
        entry_list_focus (self->entry_list, entry_listing_row);
        diary_window_push_view (diary_window, entry_edit);

        g_free (text);
        g_free (filename);
    }
}

static void
on_new_pressed (GtkWidget *widget)
{
  EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);
  GtkWindow *window;
  GtkWidget *dialog;

  window = GTK_WINDOW (diary_window_get_instance ());

  dialog = entry_name_dialog_new ();

  entry_name_dialog_open(DIARY_ENTRY_NAME_DIALOG (dialog), window, entry_name_cb, self);
}

static void
on_back_pressed (GtkWidget *widget)
{
  EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);
  entry_browser_set_content (self, NULL);
  entry_list_unfocus (self->entry_list);
}

static void
diary_header_buttons_control_init (HeaderButtonsControlInterface *iface)
{
    iface->on_enter = on_enter;
    iface->on_leave = on_leave;
    iface->on_new_pressed = on_new_pressed;
    iface->on_back_pressed = on_back_pressed;
}

G_DEFINE_TYPE_WITH_CODE (EntryBrowser, entry_browser, GTK_TYPE_BOX,
    G_IMPLEMENT_INTERFACE (DIARY_TYPE_HEADER_BUTTONS, diary_header_buttons_control_init));

static void
entry_selected_changed_cb (EntryList *list, Entry *entry, EntryBrowser *browser)
{
  if (entry == NULL) {
    entry_browser_set_content (browser, NULL);
  } else {
    GtkWidget *entry_view = entry_view_new (entry);
    g_signal_connect (entry_view, "deleted", G_CALLBACK (entry_view_deleted_cb), browser);
    entry_browser_set_content (browser, entry_view);
  }
}

static void
entry_browser_class_init (EntryBrowserClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/bjareholt/johan/simple-diary/ui/entry_browser.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryBrowser, leaflet);
  gtk_widget_class_bind_template_child (widget_class, EntryBrowser, entry_list_box);
  gtk_widget_class_bind_template_child (widget_class, EntryBrowser, content_box);
}

static void
entry_browser_init (EntryBrowser *self)
{
  self->content_box_child = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->entry_list = DIARY_ENTRY_LIST (entry_list_new ());
  gtk_box_append (self->entry_list_box, GTK_WIDGET (self->entry_list));

  g_signal_connect (self->entry_list, "selection-changed", G_CALLBACK (entry_selected_changed_cb), self);

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  adw_leaflet_set_visible_child (self->leaflet, GTK_WIDGET (self->entry_list_box));
G_GNUC_END_IGNORE_DEPRECATIONS

  g_object_set (self->content_box, "visible", FALSE, NULL);
}

GtkWidget *
entry_browser_new (void)
{
  EntryBrowser *browser = g_object_new (DIARY_TYPE_ENTRY_BROWSER, NULL);

  return GTK_WIDGET (browser);
}
