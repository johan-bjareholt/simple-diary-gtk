#pragma once

#include <gtk/gtk.h>

typedef struct _Image {
    gchar *name;
    gchar *path;
} Image;

Image *
image_new (gchar *name, gchar *path);

void
image_free (Image *image);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(Image, image_free);

#define DIARY_TYPE_IMAGE_PICKER_DIALOG (image_picker_dialog_get_type())

G_DECLARE_FINAL_TYPE (ImagePicker, image_picker_dialog, DIARY, IMAGE_PICKER_DIALOG, GtkDialog);

ImagePicker *
image_picker_dialog_new(gchar *basename);

void
image_picker_dialog_open(ImagePicker *self,
                         GtkWindow   *parent,
                         GAsyncReadyCallback callback,
                         gpointer user_data);

Image *
image_picker_dialog_open_finish (ImagePicker *self,
                                 GAsyncResult *result);
