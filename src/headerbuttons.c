#include "headerbuttons.h"
#include "window.h"

void
header_buttons_control_default_on_back_pressed (GtkWidget *widget)
{
  DiaryWindow *diary_window = diary_window_get_instance ();

  diary_window_pop_view (diary_window);
}
