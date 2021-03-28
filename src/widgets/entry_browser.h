#pragma once

#include <gtk/gtk.h>

#define DIARY_TYPE_ENTRY_BROWSER (entry_browser_get_type())

G_DECLARE_FINAL_TYPE (EntryBrowser, entry_browser, DIARY, ENTRY_BROWSER, GtkBox)

GtkWidget *
entry_browser_new (void);

void
entry_browser_set_content (EntryBrowser *self, GtkWidget *widget);
