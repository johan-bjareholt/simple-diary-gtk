#pragma once

#include <gtk/gtk.h>

#define DIARY_TYPE_IMAGE_PICKER (image_picker_get_type())

G_DECLARE_FINAL_TYPE (ImagePicker, image_picker, DIARY, IMAGE_PICKER, GtkDialog);

void
image_picker_run (gchar *basename, void (*finished_cb)(gchar *image_name, gchar *image_path, gpointer user_data), gpointer user_data);
