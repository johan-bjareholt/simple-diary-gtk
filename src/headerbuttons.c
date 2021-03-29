#include "headerbuttons.h"
#include "window.h"

G_DEFINE_INTERFACE (HeaderButtonsControl, diary_header_buttons_control, G_TYPE_OBJECT);

void
header_buttons_control_default_on_enter (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button)
{
  g_object_set (new_button, "visible", FALSE, NULL);
  g_object_set (back_button, "visible", TRUE, NULL);
  g_object_set (settings_button, "visible", FALSE, NULL);
}

void
header_buttons_control_default_on_leave (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button)
{
}

void
header_buttons_control_default_on_back_pressed (GtkWidget *widget)
{
  DiaryWindow *diary_window = diary_window_get_instance ();

  diary_window_pop_view (diary_window);
}

void
diary_header_buttons_control_default_init (HeaderButtonsControlInterface *iface)
{
  iface->on_enter = header_buttons_control_default_on_enter;
  iface->on_leave = header_buttons_control_default_on_leave;
  iface->on_back_pressed = header_buttons_control_default_on_back_pressed;
}
