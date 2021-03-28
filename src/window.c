#include <handy.h>

#include "headerbuttons.h"
#include "utils.h"
#include "window.h"
#include "widgets/entry_edit.h"
#include "widgets/entry_view.h"
#include "widgets/entry_listing.h"
#include "widgets/entry_browser.h"
#include "widgets/settings_view.h"

struct _DiaryWindow
{
  GtkApplicationWindow parent_instance;

  /* widgets */
  GtkStack *content_stack;
  GtkButton *new_button;
  GtkButton *back_button;
  GtkButton *settings_button;

  /* views */
  GList *view_stack;
};

G_DEFINE_TYPE (DiaryWindow, diary_window, GTK_TYPE_APPLICATION_WINDOW);

static DiaryWindow *diary_window_instance = NULL;

DiaryWindow *
diary_window_get_instance (void)
{
  g_assert (diary_window_instance != NULL);
  return diary_window_instance;
}

static void
diary_window_class_init (DiaryWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class, "/com/johan-bjareholt/simple-diary/ui/window.ui");
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, content_stack);
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, new_button);
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, back_button);
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, settings_button);
}

static gpointer
g_list_pop (GList **list)
{
  gpointer last_data = NULL;
  GList *last = g_list_last (*list);
  if (last != NULL) {
    last_data = last->data;
    *list = g_list_remove (*list, last_data);
  }
  return last_data;
}

static void
diary_window_update_header_buttons(DiaryWindow *self, GtkWidget *new_view, GtkWidget *old_view)
{
  HeaderButtonsControlInterface *iface;

  /* on leave */
  if (old_view != NULL) {
    iface = g_type_interface_peek (G_OBJECT_GET_CLASS (old_view), DIARY_TYPE_HEADER_BUTTONS);
    if (iface) {
      iface->on_leave (old_view, self->new_button, self->back_button, self->settings_button);
    } else {
      header_buttons_control_default_on_leave (old_view, self->new_button, self->back_button, self->settings_button);
    }
  }

  /* on enter */
  iface = g_type_interface_peek (G_OBJECT_GET_CLASS (new_view), DIARY_TYPE_HEADER_BUTTONS);
  if (iface) {
    iface->on_enter (new_view, self->new_button, self->back_button, self->settings_button);
  } else {
    header_buttons_control_default_on_enter (new_view, self->new_button, self->back_button, self->settings_button);
  }
}

void
diary_window_push_view (DiaryWindow *self, GtkWidget *new_view)
{
  GtkWidget *prev_view = NULL;
  if (self->view_stack != NULL) {
    prev_view = g_list_last (self->view_stack)->data;
  }
  /* add new view */
  self->view_stack = g_list_append (self->view_stack, new_view);
  gtk_container_add (GTK_CONTAINER (self->content_stack), new_view);

  /* set new view in focus */
  gtk_stack_set_visible_child (self->content_stack, new_view);

  /* Update header buttons */
  diary_window_update_header_buttons (self, new_view, prev_view);
}

void
diary_window_pop_view(DiaryWindow *self)
{
  GtkWidget *current_view = g_list_pop (&self->view_stack);
  GtkWidget *prev_view = GTK_WIDGET (g_list_last (self->view_stack)->data);

  diary_window_update_header_buttons (self, prev_view, current_view);

  /* remove the current view */
  gtk_widget_destroy (current_view);

  /* add previous view */
  gtk_stack_set_visible_child (self->content_stack, prev_view);
}

gboolean
back_button_pressed (GtkWidget *back_button, gpointer user_data)
{
  DiaryWindow *self = DIARY_WINDOW (user_data);
  GtkWidget *widget = GTK_WIDGET (g_list_last (self->view_stack)->data);
  HeaderButtonsControlInterface *iface;

  iface = g_type_interface_peek (G_OBJECT_GET_CLASS (widget), DIARY_TYPE_HEADER_BUTTONS);
  if (iface) {
    iface->on_back_pressed (widget);
  } else {
    header_buttons_control_default_on_back_pressed (widget);
  }

  return FALSE;
}

gboolean
new_button_pressed (GtkWidget *widget, gpointer user_data)
{
  DiaryWindow *diary_window = DIARY_WINDOW (user_data);

  Entry *entry = entry_new ();
  /* TODO: replace entry view with browser set focus */
  GtkWidget *entry_view = entry_view_new (entry);
  GtkWidget *entry_edit = entry_edit_new (entry);
  diary_window_push_view (diary_window, entry_view);
  diary_window_push_view (diary_window, entry_edit);

  return FALSE;
}

gboolean
settings_button_pressed (GtkWidget *widget, gpointer user_data)
{
  DiaryWindow *diary_window = DIARY_WINDOW (user_data);
  GtkWidget *settings_view = settings_view_new ();
  diary_window_push_view (diary_window, settings_view);
  return FALSE;
}

static void
diary_window_init (DiaryWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  diary_window_push_view (self, entry_browser_new ());
  //diary_window_push_view (self, gtk_label_new ("testing 123"));

  g_signal_connect (self->new_button, "clicked", (GCallback) new_button_pressed, self);
  g_signal_connect (self->back_button, "clicked", (GCallback) back_button_pressed, self);
  g_signal_connect (self->settings_button, "clicked", (GCallback) settings_button_pressed, self);
}

GtkApplicationWindow *
diary_window_new (GtkApplication *application)
{
  GtkApplicationWindow *window;

  window = GTK_APPLICATION_WINDOW (g_object_new (DIARY_TYPE_WINDOW, "application", application, NULL));
  g_assert (diary_window_instance == NULL);
  diary_window_instance = DIARY_WINDOW (window);

  return window;
}
