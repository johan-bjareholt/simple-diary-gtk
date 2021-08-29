#pragma once

#include <gtk/gtk.h>

#include "entry.h"
#include "entry_listing.h"

#define DIARY_TYPE_ENTRY_LIST (entry_list_get_type())

/* TODO GTK4: GtkScrolledWindow */
G_DECLARE_FINAL_TYPE (EntryList, entry_list, DIARY, ENTRY_LIST, GtkWindow)

GtkWidget * entry_list_new (void);

void entry_list_add_entry (EntryList *self, Entry *entry);
void entry_list_focus (EntryList *self, GtkListBoxRow *row);
void entry_list_unfocus (EntryList *self);
GtkListBoxRow * entry_list_find (EntryList *self, gchar *filename);
gboolean entry_list_remove (EntryList *self, Entry *entry);
