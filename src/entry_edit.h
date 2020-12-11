#pragma once

#include <gtk/gtk.h>

#include "entry.h"

#define DIARY_TYPE_ENTRY_EDIT (entry_edit_get_type())

G_DECLARE_FINAL_TYPE (EntryEdit, entry_edit, DIARY, ENTRY_EDIT, GtkBox)

GtkWidget *
entry_edit_new (Entry *entry);
