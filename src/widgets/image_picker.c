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

  GtkCheckButton *file_radio_button;
  GtkButton *file_button;

  GtkCheckButton *clipboard_radio_button;

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
      "/com/johan-bjareholt/simple-diary/ui/image_picker.ui");

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, add_button);
  gtk_widget_class_bind_template_child (widget_class, ImagePicker, cancel_button);

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, name_entry);

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, file_radio_button);
  gtk_widget_class_bind_template_child (widget_class, ImagePicker, file_button);

  gtk_widget_class_bind_template_child (widget_class, ImagePicker, clipboard_radio_button);
}

static void
response_ok_cb (GtkWidget *widget, gpointer user_data)
{
  GtkDialog *self = GTK_DIALOG (user_data);
  gtk_dialog_response (self, GTK_RESPONSE_OK);
}

static void
radio_button_pressed (GtkWidget *widget, gpointer user_data)
{
  ImagePicker *self = DIARY_IMAGE_PICKER (user_data);
  gtk_widget_set_sensitive (GTK_WIDGET (self->file_button), GTK_WIDGET (self->file_radio_button) == widget);
}

static void
image_picker_init (ImagePicker *self)
{
  //GtkDialog *dialog = GTK_DIALOG (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->name_entry, "activate", (GCallback) response_ok_cb, self);

  g_signal_connect (self->file_radio_button, "toggled", (GCallback) radio_button_pressed, self);
  g_signal_connect (self->clipboard_radio_button, "toggled", (GCallback) radio_button_pressed, self);
}

static gboolean
copy_file (gchar *src_path, gchar *dest_path, GError **err)
{
  gboolean ret;
  GFile *src_file, *dest_file;

  src_file = g_file_new_for_path (src_path);
  dest_file = g_file_new_for_path (dest_path);

  ret = g_file_copy (src_file,
                 dest_file,
                 G_FILE_COPY_TARGET_DEFAULT_PERMS,
                 NULL,
                 NULL,
                 NULL,
                 err);

  g_object_unref (src_file);
  g_object_unref (dest_file);

  return ret;
}

static gboolean
image_picker_save_from_file (ImagePicker *image_picker, gchar *basename,
                             gchar **image_name, gchar **image_path_relative)
{
  gchar *src_path;
  gchar *image_extension;
  gchar *target_dir_absolute;
  gchar *target_dir_relative;
  gchar *image_path_absolute;
  GError *err = NULL;

  /* TODO GTK4: fix file chooser */
  src_path = g_strdup ("test");
  /*
  src_path = gtk_file_chooser_get_filename (
      GTK_FILE_CHOOSER (image_picker->file_button));
  if (src_path == NULL) {
    utils_error_dialog ("No file chosen, ignoring\n");
    goto error;
  }
  */
  image_extension = utils_get_file_extension (src_path);

  /* relative path */
  target_dir_relative = utils_get_photos_folder (basename, FALSE);
  *image_path_relative = g_strdup_printf ("%s/%s%s", target_dir_relative, *image_name,
      image_extension);
  g_free (target_dir_relative);

  /* absolute path */
  target_dir_absolute = utils_get_photos_folder (basename, TRUE);
  image_path_absolute = g_strdup_printf ("%s/%s%s", target_dir_absolute, *image_name,
      image_extension);
  g_free (target_dir_absolute);

  if (!copy_file (src_path, image_path_absolute, &err)) {
    utils_error_dialog ("Failed to copy image: %s\n", err->message);
    goto error;
  }

  return TRUE;

error:
  if (*image_path_relative != NULL) {
    g_free (*image_path_relative);
    *image_path_relative = NULL;
  }

  return FALSE;
}

static gboolean
image_picker_save_from_clipboard (gchar *basename,
                                 gchar **image_name, gchar **image_path_relative)
{
  //GdkClipboard *clipboard;
  //GdkPixbuf *pixbuf;
  gchar *target_dir_absolute;
  gchar *target_dir_relative;
  gchar *image_path_absolute;
  GError *err = NULL;
  gboolean ret = FALSE;

  /* relative path */
  target_dir_relative = utils_get_photos_folder (basename, FALSE);
  *image_path_relative = g_strdup_printf ("%s/%s.jpg", target_dir_relative, *image_name);
  g_free (target_dir_relative);

  /* absolute path */
  target_dir_absolute = utils_get_photos_folder (basename, TRUE);
  image_path_absolute = g_strdup_printf ("%s/%s.jpg", target_dir_absolute, *image_name);
  g_free (target_dir_absolute);

  /* TODO GTK4: Fix */
  /*
  clipboard = gdk_display_get_clipboard (gdk_display_get_default ());

  pixbuf = gdk_clipboard_wait_for_image (clipboard);

  if (pixbuf == NULL) {
      utils_error_dialog ("Could not fetch image from clipboard\n");
      goto out;
  }
  gdk_pixbuf_save (pixbuf, image_path_absolute, "jpeg", &err, "quality", "100", NULL);
  */

  ret = TRUE;

//out:
  g_free (image_path_absolute);
  if (err != NULL) {
    utils_error_dialog ("Failed to save image: %s\n", err->message);
  }
  return ret;
}

typedef enum _RadioButton {
  RADIO_BUTTON_UNKNOWN,
  RADIO_BUTTON_FILE,
  RADIO_BUTTON_CLIPBOARD,
} RadioButton;

static RadioButton
get_selected_radiobutton (ImagePicker *image_picker) {
  gboolean active = FALSE;

  g_object_get (image_picker->file_radio_button, "active", &active, NULL);
  if (active) {
    return RADIO_BUTTON_FILE;
  }

  g_object_get (image_picker->clipboard_radio_button, "active", &active, NULL);
  if (active) {
    return RADIO_BUTTON_CLIPBOARD;
  }

  g_assert_not_reached ();
  return RADIO_BUTTON_UNKNOWN;
}

static void
image_picker_response (GtkDialog *dialog, int response_id, gpointer user_data)
{
  ImagePicker *image_picker = DIARY_IMAGE_PICKER (dialog);
  GtkEntryBuffer *buffer;
  gchar *image_name = NULL;
  gchar *image_path_relative = NULL;
  gboolean ret = FALSE;

  switch (response_id) {
    case GTK_RESPONSE_OK:
      break;
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_NONE:
    case GTK_RESPONSE_DELETE_EVENT:
      goto out;
    default:
      g_printerr ("Invalid GTK_RESPONSE enum: %d\n", response_id);
      g_assert_not_reached ();
      exit (1);
  }

  buffer = gtk_entry_get_buffer (image_picker->name_entry);
  image_name = g_strdup (gtk_entry_buffer_get_text (buffer));

  switch (get_selected_radiobutton (image_picker)) {
    case RADIO_BUTTON_FILE:
      ret = image_picker_save_from_file (image_picker, image_picker->basename,
                                         &image_name, &image_path_relative);
      break;
    case RADIO_BUTTON_CLIPBOARD:
      ret = image_picker_save_from_clipboard (image_picker->basename,
                                              &image_name, &image_path_relative);
      break;
    default:
      g_assert_not_reached ();
      g_printerr ("Invalid radio button type\n");
      exit (1);
  }

out:
  gtk_window_destroy (GTK_WINDOW (image_picker));
  if (ret == FALSE) {
    g_free (image_name);
    g_free (image_path_relative);
  } else {
    // TODO: call callback
  }

}

/* TODO: Fix so links are not absolute but relative */
/* TODO: Make "file picker" inactive when clipboard is chosen */
void
image_picker_run (gchar *basename, void (*finished_cb)(gchar *image_name, gchar *image_path, gpointer user_data), gpointer user_data)
{
  ImagePicker *image_picker;
  GtkWindow *window = GTK_WINDOW (diary_window_get_instance ());

  image_picker = g_object_new (DIARY_TYPE_IMAGE_PICKER, NULL);
  image_picker->basename = basename;
  image_picker->finished_cb = finished_cb;
  image_picker->user_data = user_data;

  g_signal_connect (image_picker,
                    "response",
                    G_CALLBACK (image_picker_response),
                    NULL);

  gtk_window_set_transient_for (GTK_WINDOW (image_picker), window);
  gtk_widget_show (GTK_WIDGET (image_picker));
}
