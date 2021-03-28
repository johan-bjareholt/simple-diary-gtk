#include <handy.h>

#include "headerbuttons.h"
#include "entry_browser.h"
#include "entry_list.h"
#include "entry_view.h"
#include "entry.h"

#include "entry.h"

struct _EntryBrowser
{
  GtkBox parent_instance;

  HdyLeaflet *leaflet;
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
on_enter (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button)
{
  EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);

  self->new_button = new_button;
  self->back_button = back_button;

  g_object_set (settings_button, "visible", TRUE, NULL);
  update_header_buttons (self);

  /* hide content box if nothing in stack */
  /*
  if (g_list_length (self->view_stack) == 0) {
    //g_object_set (self->content_box, "visible", FALSE, NULL);
    //hdy_leaflet_set_visible_child (self->leaflet, GTK_WIDGET (self->entry_list_container));
    //gtk_widget_set_hexpand (GTK_WIDGET (self->entry_list_container), TRUE);
  } else {
    //g_object_set (self->content_box, "visible", TRUE, NULL);
    //hdy_leaflet_set_visible_child (self->leaflet, GTK_WIDGET (self->content_box));
    //gtk_widget_set_hexpand (GTK_WIDGET (self->entry_list_container), FALSE);
  }
  */
}

static void
on_leave (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button)
{
    EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);
    g_print ("leave %p\n", self);
}

static void
on_back_pressed (GtkWidget *widget)
{
  EntryBrowser *self = DIARY_ENTRY_BROWSER (widget);
  entry_browser_set_content (self, NULL);
}

static void
diary_header_buttons_control_init (HeaderButtonsControlInterface *iface)
{
    iface->on_enter = on_enter;
    iface->on_leave = on_leave;
    iface->on_back_pressed = on_back_pressed;
}

G_DEFINE_TYPE_WITH_CODE (EntryBrowser, entry_browser, GTK_TYPE_BOX,
    G_IMPLEMENT_INTERFACE (DIARY_TYPE_HEADER_BUTTONS, diary_header_buttons_control_init));

static void
on_content_destroyed (GtkWidget *content, gpointer user_data)
{
  EntryBrowser *browser = DIARY_ENTRY_BROWSER (user_data);
  entry_browser_set_content (browser, NULL);
}

void
entry_browser_set_content (EntryBrowser *self, GtkWidget *widget)
{
  // Remove old content
  if (self->content_box_child != NULL) {
    gtk_container_remove (GTK_CONTAINER (self->content_box), self->content_box_child);
  }

  // Add new content if there is any
  if (widget == NULL) {
    self->content_box_child = NULL;
    g_object_set (self->content_box, "visible", FALSE, NULL);
    gtk_widget_set_hexpand (GTK_WIDGET (self->entry_list_box), TRUE);
  } else {
    self->content_box_child = widget;
    gtk_container_add (GTK_CONTAINER (self->content_box), self->content_box_child);
    g_signal_connect (widget, "destroy", G_CALLBACK (on_content_destroyed), self);
    g_object_set (self->content_box, "visible", TRUE, NULL);
    hdy_leaflet_set_visible_child (self->leaflet, GTK_WIDGET (self->content_box));
    gtk_widget_set_hexpand (GTK_WIDGET (self->entry_list_box), FALSE);
  }

  update_header_buttons (self);
}

static void
entry_selected_changed_cb (EntryList *list, Entry *entry, EntryBrowser *browser)
{
  g_print ("list: %p\n", list);
  g_print ("entry: %p\n", entry);
  if (entry == NULL) {
    entry_browser_set_content (browser, NULL);
  } else {
    entry_browser_set_content (browser, GTK_WIDGET (entry_view_new (entry)));
  }
}

static void
entry_browser_class_init (EntryBrowserClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/johan-bjareholt/simple-diary/ui/entry_browser.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryBrowser, leaflet);
  gtk_widget_class_bind_template_child (widget_class, EntryBrowser, entry_list_box);
  gtk_widget_class_bind_template_child (widget_class, EntryBrowser, content_box);
}

static void
entry_browser_init (EntryBrowser *self)
{
  GtkWidget *entry_list;

  self->content_box_child = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  entry_list = entry_list_new ();
  gtk_container_add (GTK_CONTAINER (self->entry_list_box), entry_list);

  g_signal_connect (entry_list, "selection-changed", G_CALLBACK (entry_selected_changed_cb), self);

  hdy_leaflet_set_visible_child (self->leaflet, GTK_WIDGET (self->entry_list_box));

  g_object_set (self->content_box, "visible", FALSE, NULL);
}

GtkWidget *
entry_browser_new (void)
{
  EntryBrowser *browser = g_object_new (DIARY_TYPE_ENTRY_BROWSER, NULL);

  return GTK_WIDGET (browser);
}
