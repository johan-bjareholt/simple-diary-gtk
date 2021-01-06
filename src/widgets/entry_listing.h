#pragma once

#include <gtk/gtk.h>

#include "entry.h"

#define DIARY_TYPE_ENTRY_LISTING (entry_listing_get_type())

G_DECLARE_FINAL_TYPE (EntryListing, entry_listing, DIARY, ENTRY_LISTING, GtkBox)

GtkWidget *
entry_listing_new (Entry *entry);
