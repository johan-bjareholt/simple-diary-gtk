#pragma once

#include <gtk/gtk.h>

#include "entry.h"
#include "entry_listing.h"

#define DIARY_TYPE_ENTRY_LIST (entry_list_get_type())

G_DECLARE_FINAL_TYPE (EntryList, entry_list, DIARY, ENTRY_LIST, GtkScrolledWindow)

GtkWidget * entry_list_new (void);

void entry_list_add_entry (EntryList *self, Entry *entry, gboolean focus);
EntryListing * entry_list_find (EntryList *self, gchar *filename);
