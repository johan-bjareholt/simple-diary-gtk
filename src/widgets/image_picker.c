#include <gdk-pixbuf/gdk-pixbuf.h>

#include "image_picker.h"
#include "utils.h"
#include "window.h"

struct _ImagePicker
{
  GtkWindow parent_instance;

  /* widgets */
  GtkButton *add_button;
  GtkButton *cancel_button;

  GtkEntry *name_entry;

  GtkButton *file_button;
  GtkButton *clipboard_button;
  gchar *picked_image_path;

  GtkPicture *image_preview;
  GdkTexture *image_texture;

  /* arguments */
  gchar *basename;
  void (*finished_cb)(gchar *image_name, gchar *image_path, gpointer user_data);
  gpointer user_data;
};

G_DEFINE_TYPE (ImagePicker, image_picker_dialog, GTK_TYPE_WINDOW);

static void
image_picker_dialog_class_init (ImagePickerClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/image_picker.ui");

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, add_button);
  gtk_widget_class_bind_template_child (widget_class, ImagePicker, cancel_button);

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, name_entry);

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, file_button);
  gtk_widget_class_bind_template_child (widget_class, ImagePicker, clipboard_button);

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, image_preview);

}

static void select_file (GtkWidget *button, ImagePicker *dialog);
static void select_clipboard (GtkWidget *button, ImagePicker *dialog);

static void
image_picker_dialog_init (ImagePicker *self)
{
  self->picked_image_path = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->file_button, "clicked", (GCallback) select_file, self);
  g_signal_connect (self->clipboard_button, "clicked", (GCallback) select_clipboard, self);
}

static void
open_file_cb(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
    GtkFileDialog *file_dialog = GTK_FILE_DIALOG (source_object);
    ImagePicker *dialog = DIARY_IMAGE_PICKER_DIALOG (user_data);
    GdkPixbuf *pixbuf;
    GdkTexture *texture;
    g_autoptr(GFile) file = NULL;
    GError *err = NULL;

    file = gtk_file_dialog_open_finish (file_dialog, res, &err);
    if (file == NULL) {
        g_print("Failed to select file: %s\n", err->message);
        return;
    }

    pixbuf = gdk_pixbuf_new_from_file (g_file_peek_path (file), &err);
    if (pixbuf == NULL) {
      utils_error_dialog (GTK_WIDGET (dialog), "Failed to load image: %s\n", err->message);
      g_clear_error (&err);
      return;
    }

    texture = gdk_texture_new_for_pixbuf (pixbuf);
    if (dialog->image_texture != NULL) {
      g_object_unref (dialog->image_texture);
    }
    dialog->image_texture = texture;
    gtk_picture_set_paintable (dialog->image_preview, GDK_PAINTABLE (texture));
}

/* Called when pressing "file_button" */
static void
select_file (GtkWidget *button, ImagePicker *dialog)
{
    GtkFileDialog *file_dialog;

    file_dialog = gtk_file_dialog_new();
    gtk_file_dialog_open (file_dialog,
                          GTK_WINDOW (dialog),
                          NULL,
                          open_file_cb,
                          dialog);
}

static void
select_clipboard_cb (GObject *source_object, GAsyncResult *result, gpointer user_data) {
  ImagePicker *dialog = DIARY_IMAGE_PICKER_DIALOG (user_data);
  GError *err = NULL;
  GdkClipboard *clipboard = GDK_CLIPBOARD (source_object);
  GdkTexture* texture;

  texture = gdk_clipboard_read_texture_finish (clipboard, result, &err);
  if (texture == NULL) {
      goto error;
  }

  if (dialog->image_texture) {
    g_object_unref (dialog->image_texture);
  }
  dialog->image_texture = texture;
  gtk_picture_set_paintable (dialog->image_preview, GDK_PAINTABLE (texture));

error:
  if (err != NULL) {
    utils_error_dialog (GTK_WIDGET (dialog), "Failed to save image: %s\n", err->message);
    g_clear_error (&err);
  }
}

static void
select_clipboard (GtkWidget *button, ImagePicker *dialog)
{
  GdkClipboard *clipboard;

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (dialog));
  if (clipboard == NULL) {
    utils_error_dialog (GTK_WIDGET (dialog), "Failed to get clipboard");
    return;
  }

  gdk_clipboard_read_texture_async (clipboard, NULL, select_clipboard_cb, dialog);
}

ImagePicker *
image_picker_dialog_new(gchar *basename)
{
  ImagePicker *dialog;

  dialog = g_object_new (DIARY_TYPE_IMAGE_PICKER_DIALOG, NULL);
  dialog->basename = g_strdup (basename);

  return dialog;
}

Image *
image_new (gchar *name, gchar *path)
{
    Image *image = g_malloc0 (sizeof (Image));
    image->name = name;
    image->path = path;
    return image;
}

void
image_free (Image *image)
{
    g_free (image->name);
    g_free (image->path);
    g_free (image);
}

static Image *
save_image (ImagePicker *dialog)
{
  g_autofree gchar *target_dir_absolute = NULL;
  g_autofree gchar *target_dir_relative = NULL;
  g_autofree gchar *image_path_absolute = NULL;
  g_autofree gchar *image_path_relative = NULL;
  g_autofree gchar *image_name = NULL;
  GtkEntryBuffer *buffer;

  buffer = gtk_entry_get_buffer (dialog->name_entry);
  image_name = g_strdup (gtk_entry_buffer_get_text (buffer));

  /* relative path */
  g_print ("basename: %s\n", dialog->basename);
  target_dir_relative = utils_get_photos_folder (dialog->basename, FALSE);
  image_path_relative = g_strdup_printf ("%s/%s.png", target_dir_relative, image_name);

  /* absolute path */
  target_dir_absolute = utils_get_photos_folder (dialog->basename, TRUE);
  image_path_absolute = g_strdup_printf ("%s/%s.png", target_dir_absolute, image_name);

  if (dialog->image_texture == NULL) {
      utils_error_dialog (GTK_WIDGET (dialog), "No image selected\n");
      return NULL;
  }

  /* TODO: save as jpg instead of png
   * should be possible with gdk_texture_download together with gdkpixbuf somehow */
  if (!gdk_texture_save_to_png (dialog->image_texture, image_path_absolute)) {
      utils_error_dialog (GTK_WIDGET (dialog), "Could not save image to png file\n");
      return NULL;
  }

  return image_new (g_steal_pointer (&image_name), g_steal_pointer (&image_path_relative));
}

static void
dialog_response (GtkWidget *widget,
                 gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    ImagePicker *dialog = DIARY_IMAGE_PICKER_DIALOG (g_task_get_source_object (task));
    Image *image = save_image(dialog);
    if (image) {
        gtk_window_close (GTK_WINDOW (dialog));
        g_task_return_pointer (task, image, (GDestroyNotify) image_free);
    } else {
        g_task_return_pointer (task, NULL, NULL);
    }
}

static void
dialog_cancel (GtkWidget *widget,
               gpointer user_data)
{
    GTask *task = G_TASK (user_data);
    ImagePicker *dialog = DIARY_IMAGE_PICKER_DIALOG (g_task_get_source_object (task));
    gtk_window_close (GTK_WINDOW (dialog));
    g_task_return_pointer (task, NULL, NULL);
}

void
image_picker_dialog_open(ImagePicker *self,
                         GtkWindow   *parent,
                         GAsyncReadyCallback callback,
                         gpointer user_data)
{
  GTask *task;

  task = g_task_new (self, FALSE, callback, user_data);
  g_task_set_check_cancellable (task, FALSE);
  g_task_set_source_tag (task, image_picker_dialog_open);
  g_task_set_task_data (task, user_data, NULL);

  g_signal_connect (self->name_entry, "activate", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->add_button, "clicked", G_CALLBACK (dialog_response), task);
  g_signal_connect (self->cancel_button, "clicked", G_CALLBACK (dialog_cancel), task);

  gtk_window_set_transient_for (GTK_WINDOW (self), parent);
  gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
}

Image *
image_picker_dialog_open_finish (ImagePicker *self,
                                 GAsyncResult *result)
{
  gtk_window_destroy (GTK_WINDOW (self));

  return g_task_propagate_pointer (G_TASK (result), NULL);
}
