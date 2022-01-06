#include <gdk-pixbuf/gdk-pixbuf.h>

#include "image_picker.h"
#include "utils.h"
#include "window.h"

struct _ImagePicker
{
  GtkDialog parent_instance;

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

G_DEFINE_TYPE (ImagePicker, image_picker, GTK_TYPE_DIALOG);

static void
image_picker_class_init (ImagePickerClass *klass)
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

static void
response_ok_cb (GtkWidget *widget, gpointer user_data)
{
  GtkDialog *self = GTK_DIALOG (user_data);
  gtk_dialog_response (self, GTK_RESPONSE_OK);
}

static void select_file (GtkWidget *button, ImagePicker *image_picker);
static void select_clipboard (GtkWidget *button, ImagePicker *image_picker);

static void
image_picker_init (ImagePicker *self)
{
  //GtkDialog *dialog = GTK_DIALOG (self);
  self->picked_image_path = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->name_entry, "activate", (GCallback) response_ok_cb, self);

  g_signal_connect (self->file_button, "clicked", (GCallback) select_file, self);
  g_signal_connect (self->clipboard_button, "clicked", (GCallback) select_clipboard, self);
}

static void
select_file_cb (GtkDialog *dialog, int response_id, gpointer user_data)
{
  ImagePicker *image_picker = DIARY_IMAGE_PICKER (user_data);

  if (response_id == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser;
    GFile *file;
    GdkPixbuf *pixbuf;
    GdkTexture *texture;
    GError *err = NULL;

    chooser = GTK_FILE_CHOOSER (dialog);
    file = gtk_file_chooser_get_file (chooser);

    pixbuf = gdk_pixbuf_new_from_file (g_file_peek_path (file), &err);
    if (pixbuf == NULL) {
      utils_error_dialog ("Failed to load image: %s\n", err->message);
      g_clear_error (&err);
      return;
    }

    texture = gdk_texture_new_for_pixbuf (pixbuf);
    if (image_picker->image_texture != NULL) {
      g_object_unref (image_picker->image_texture);
    }
    image_picker->image_texture = texture;
    gtk_picture_set_paintable (image_picker->image_preview, GDK_PAINTABLE (texture));
  }

  gtk_window_destroy (GTK_WINDOW (dialog));
}

/* Called when pressing "file_button" */
static void
select_file (GtkWidget *button, ImagePicker *image_picker)
{
  GtkWidget *dialog;

  dialog = gtk_file_chooser_dialog_new ("Select image file",
                                        GTK_WINDOW (image_picker),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "Select",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (select_file_cb),
                    image_picker);

  gtk_widget_show (dialog);
}

static void
select_clipboard_cb (GObject *source_object, GAsyncResult *result, gpointer user_data) {
  ImagePicker *image_picker = DIARY_IMAGE_PICKER (user_data);
  GError *err = NULL;
  GdkClipboard *clipboard = GDK_CLIPBOARD (source_object);
  GdkTexture* texture;

  texture = gdk_clipboard_read_texture_finish (clipboard, result, &err);
  if (texture == NULL) {
      goto error;
  }

  if (image_picker->image_texture) {
    g_object_unref (image_picker->image_texture);
  }
  image_picker->image_texture = texture;
  gtk_picture_set_paintable (image_picker->image_preview, GDK_PAINTABLE (texture));

error:
  if (err != NULL) {
    utils_error_dialog ("Failed to save image: %s\n", err->message);
    g_clear_error (&err);
  }
}

static void
select_clipboard (GtkWidget *button, ImagePicker *image_picker)
{
  GdkClipboard *clipboard;

  clipboard = gtk_widget_get_clipboard (GTK_WIDGET (image_picker));
  if (clipboard == NULL) {
    utils_error_dialog ("Failed to get clipboard");
    return;
  }

  gdk_clipboard_read_texture_async (clipboard, NULL, select_clipboard_cb, image_picker);
}

static gboolean
save_image (ImagePicker *image_picker)
{
  GError *err = NULL;
  gchar *target_dir_absolute = NULL;
  gchar *target_dir_relative = NULL;
  gchar *image_path_absolute = NULL;
  gchar *image_path_relative = NULL;
  GtkEntryBuffer *buffer;
  gchar *image_name = NULL;

  buffer = gtk_entry_get_buffer (image_picker->name_entry);
  image_name = g_strdup (gtk_entry_buffer_get_text (buffer));

  /* relative path */
  g_print ("basename: %s\n", image_picker->basename);
  target_dir_relative = utils_get_photos_folder (image_picker->basename, FALSE);
  image_path_relative = g_strdup_printf ("%s/%s.png", target_dir_relative, image_name);

  /* absolute path */
  target_dir_absolute = utils_get_photos_folder (image_picker->basename, TRUE);
  image_path_absolute = g_strdup_printf ("%s/%s.png", target_dir_absolute, image_name);

  /* TODO: save as jpg instead of png
   * should be possible with gdk_texture_download together with gdkpixbuf somehow */
  if (!gdk_texture_save_to_png (image_picker->image_texture, image_path_absolute)) {
      utils_error_dialog ("Could not save image to png file\n");
      goto error;
  }

  image_picker->finished_cb(image_name, image_path_relative, image_picker->user_data);

  gtk_window_destroy (GTK_WINDOW (image_picker));

error:
  if (err != NULL) {
    utils_error_dialog ("Failed to save image: %s\n", err->message);
    g_clear_error (&err);
  }

  g_free (target_dir_relative);
  g_free (target_dir_absolute);
  g_free (image_path_relative);
  g_free (image_path_absolute);
  g_free (image_name);

  return TRUE;
}

/* Called when the Insert button is pressed in the ImagePicker dialog */
static void
image_picker_response (GtkDialog *dialog, int response_id, gpointer user_data)
{
  ImagePicker *image_picker = DIARY_IMAGE_PICKER (dialog);

  if (response_id != GTK_RESPONSE_OK) {
    /* cancel */
    gtk_window_destroy (GTK_WINDOW (image_picker));
  } else {
    if (save_image (image_picker)) {
      gtk_window_destroy (GTK_WINDOW (image_picker));
    }
  }

  g_free (image_picker->basename);
}

void
image_picker_run (gchar *basename, void (*finished_cb)(gchar *image_name, gchar *image_path, gpointer user_data), gpointer user_data)
{
  ImagePicker *image_picker;
  GtkWindow *window;

  window = GTK_WINDOW (diary_window_get_instance ());
  image_picker = g_object_new (DIARY_TYPE_IMAGE_PICKER, NULL);
  image_picker->basename = g_strdup (basename);
  image_picker->finished_cb = finished_cb;
  image_picker->user_data = user_data;

  g_signal_connect (image_picker,
                    "response",
                    G_CALLBACK (image_picker_response),
                    NULL);

  gtk_window_set_transient_for (GTK_WINDOW (image_picker), window);
  gtk_widget_show (GTK_WIDGET (image_picker));
}
