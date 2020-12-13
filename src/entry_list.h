#pragma once

#include <gtk/gtk.h>

#define DIARY_TYPE_ENTRY_LIST (entry_list_get_type())

G_DECLARE_FINAL_TYPE (EntryList, entry_list, DIARY, ENTRY_LIST, GtkScrolledWindow)

GtkWidget * entry_list_new (void);