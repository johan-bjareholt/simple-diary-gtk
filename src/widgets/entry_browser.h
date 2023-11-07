#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>

#define DIARY_TYPE_ENTRY_BROWSER (entry_browser_get_type())

G_DECLARE_FINAL_TYPE (EntryBrowser, entry_browser, DIARY, ENTRY_BROWSER, AdwBreakpointBin)

GtkWidget *
entry_browser_new (void);
