#pragma once

#include <gtk/gtk.h>

#define DIARY_TYPE_HEADER_BUTTONS diary_header_buttons_control_get_type ()
G_DECLARE_INTERFACE (HeaderButtonsControl, diary_header_buttons_control, DIARY, HEADER_BUTTONS, GObject)

struct _HeaderButtonsControlInterface
{
  GTypeInterface parent_iface;

  void (*on_enter) (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button);
  void (*on_leave) (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button);
  void (*on_back_pressed) (GtkWidget *widget);
};

void
header_buttons_control_default_on_enter (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button);

void
header_buttons_control_default_on_leave (GtkWidget *widget, GtkButton *new_button, GtkButton *back_button, GtkButton *settings_button);

void
header_buttons_control_default_on_back_pressed (GtkWidget *widget);
