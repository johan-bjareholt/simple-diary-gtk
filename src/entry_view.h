#pragma once

#include <gtk/gtk.h>

#include "entry.h"

#define DIARY_TYPE_ENTRY_VIEW (entry_view_get_type())

G_DECLARE_FINAL_TYPE (EntryView, entry_view, DIARY, ENTRY_VIEW, GtkBox)

GtkWidget *
entry_view_new (Entry *entry);
