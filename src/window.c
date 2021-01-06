#include "utils.h"
#include "window.h"
#include "widgets/entry_edit.h"
#include "widgets/entry_view.h"
#include "widgets/entry_listing.h"
#include "widgets/entry_list.h"

struct _DiaryWindow
{
  GtkApplicationWindow parent_instance;

  /* widgets */
  GtkStack *content_box;
  GtkButton *new_button;
  GtkButton *back_button;

  /* views */
  GList *view_stack;
};

G_DEFINE_TYPE (DiaryWindow, diary_window, GTK_TYPE_APPLICATION_WINDOW)

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
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, new_button);
  gtk_widget_class_bind_template_child (widget_class, DiaryWindow, back_button);
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
diary_window_update_button_text(DiaryWindow *self)
{
  /* set label depending on button history */
  if (g_list_length (self->view_stack) == 1) {
    g_object_set (self->new_button, "visible", TRUE, NULL);
    g_object_set (self->back_button, "visible", FALSE, NULL);
  } else {
    g_object_set (self->new_button, "visible", FALSE, NULL);
    g_object_set (self->back_button, "visible", TRUE, NULL);
  }
}

void
diary_window_push_view (DiaryWindow *self, GtkWidget *new_view)
{
  /* remove the old view */
  if (g_list_length (self->view_stack) > 0) {
    GtkWidget *last_view = g_list_last (self->view_stack)->data;
    g_object_ref (last_view);
    gtk_container_remove (GTK_CONTAINER (self->content_box), last_view);
  }
  /* add new view */
  g_object_ref (new_view);
  self->view_stack = g_list_append (self->view_stack, new_view);
  // TODO: replace container add with stack_set_visible_child
  gtk_container_add (GTK_CONTAINER (self->content_box), new_view);

  diary_window_update_button_text (self);
}

void
diary_window_pop_view(DiaryWindow *self)
{
  /* remove the old view */
  GtkWidget *last_view = g_list_pop (&self->view_stack);
  gtk_container_remove (GTK_CONTAINER (self->content_box), last_view);
  g_object_unref (last_view);
  /* add new view */
  GtkWidget *new_view = GTK_WIDGET (g_list_last (self->view_stack)->data);
  gtk_container_add (GTK_CONTAINER (self->content_box), new_view);

  diary_window_update_button_text (self);
}

gboolean
back_button_pressed (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  DiaryWindow *diary_window = DIARY_WINDOW (user_data);

  diary_window_pop_view (diary_window);

  return FALSE;
}

gboolean
new_button_pressed (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  DiaryWindow *diary_window = DIARY_WINDOW (user_data);

  Entry *entry = entry_new ();
  GtkWidget *entry_view = entry_view_new (entry);
  GtkWidget *entry_edit = entry_edit_new (entry);
  diary_window_push_view (diary_window, entry_view);
  diary_window_push_view (diary_window, entry_edit);

  return FALSE;
}

static void
diary_window_init (DiaryWindow *self)
{
  GtkWidget *entry_list;

  gtk_widget_init_template (GTK_WIDGET (self));

  entry_list = entry_list_new (); //load_entry_list ();
  diary_window_push_view (self, entry_list);

  g_signal_connect (self->new_button, "button-release-event", (GCallback) new_button_pressed, self);
  g_signal_connect (self->back_button, "button-release-event", (GCallback) back_button_pressed, self);

  gtk_widget_show_all (GTK_WIDGET (self));

  diary_window_update_button_text (self);
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
