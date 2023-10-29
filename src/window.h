#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>

#define DIARY_TYPE_WINDOW (diary_window_get_type())

G_DECLARE_FINAL_TYPE (DiaryWindow, diary_window, DIARY, WINDOW, AdwApplicationWindow)

DiaryWindow *
diary_window_get_instance (void);

void
diary_window_push_view (DiaryWindow *self, GtkWidget *new_view);

void
diary_window_pop_view (DiaryWindow *self);

GtkApplicationWindow *
diary_window_new (GtkApplication *application);
