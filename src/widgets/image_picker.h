#pragma once

#include <gtk/gtk.h>

#define DIARY_TYPE_IMAGE_PICKER_DIALOG (image_picker_dialog_get_type())

G_DECLARE_FINAL_TYPE(ImagePicker, image_picker_dialog, DIARY, IMAGE_PICKER_DIALOG, GtkWindow);

ImagePicker *image_picker_dialog_new(gchar *basename);

void image_picker_dialog_open(ImagePicker *self, GtkWindow *parent);

// Signals:
// "image-selected" (gchar *name, gchar *path)
// "cancelled" ()
