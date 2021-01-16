#pragma once

#include <gtk/gtk.h>

#include "entry.h"

G_BEGIN_DECLS

#define DIARY_TYPE_SETTINGS_VIEW (settings_view_get_type())

#define DIARY_TYPE_IS_SETTINGS_VIEW(object) \
    (G_OBJECT_TYPE (object) == DIARY_TYPE_SETTINGS_VIEW)

G_DECLARE_FINAL_TYPE (SettingsView, settings_view, DIARY, SETTINGS_VIEW, GtkBox)

GtkWidget *
settings_view_new ();

G_END_DECLS
