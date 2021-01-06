#include "image_picker.h"
#include "utils.h"

struct _ImagePicker
{
  GtkDialog parent_instance;

  /* widgets */
  GtkButton *add_button;
  GtkButton *cancel_button;

  GtkEntry *name_entry;

  GtkRadioButton *file_radio_button;
  GtkFileChooserButton *file_button;

  GtkRadioButton *clipboard_radio_button;
};

G_DEFINE_TYPE (ImagePicker, image_picker, GTK_TYPE_DIALOG);

static void
image_picker_class_init (ImagePickerClass *klass)
{
  //GObjectClass *object_class = G_OBJECT_CLASS (klass);
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

  src_path = gtk_file_chooser_get_filename (
      GTK_FILE_CHOOSER (image_picker->file_button));
  if (src_path == NULL) {
    /* TODO: display warning to user */
    g_print ("No file chosen, ignoring\n");
    goto error;
  }
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
    g_print ("Failed to copy image: %s\n", err->message);
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
  GtkClipboard *clipboard;
  GdkPixbuf *pixbuf;
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

  clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);

  pixbuf = gtk_clipboard_wait_for_image (clipboard);

  if (pixbuf == NULL) {
      goto out;
  }
  gdk_pixbuf_save (pixbuf, image_path_absolute, "jpeg", &err, "quality", "100", NULL);

  ret = TRUE;

out:
  g_free (image_path_absolute);
  if (err != NULL) {
    g_printerr ("Failed to save image: %s\n", err->message);
  }
  return ret;
}

/* TODO: Fix so links are not absolute but relative */
/* TODO: Make "file picker" inactive when clipboard is chosen */
gboolean
image_picker_run (gchar *basename, gchar **image_name, gchar **image_path_relative)
{
  gint result;
  ImagePicker *image_picker;
  gboolean active;
  gboolean ret = FALSE;

  g_assert (image_name != NULL);
  g_assert (image_path_relative != NULL);

  image_picker = g_object_new (DIARY_TYPE_IMAGE_PICKER, NULL);

  result = gtk_dialog_run (GTK_DIALOG (image_picker));

  *image_name = NULL;
  *image_path_relative = NULL;

  switch (result) {
    case GTK_RESPONSE_OK:
      break;
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_NONE:
    case GTK_RESPONSE_DELETE_EVENT:
      goto out;
    default:
      g_print ("Invalid GTK_RESPONSE enum: %d\n", result);
      g_assert_not_reached ();
      goto out;
  }

  *image_name = g_strdup (gtk_entry_get_text (image_picker->name_entry));

  g_object_get (image_picker->file_radio_button, "active", &active, NULL);
  if (active) {
    g_print ("File active\n");
    ret = image_picker_save_from_file (image_picker, basename, image_name,
                                       image_path_relative);
  }
  g_object_get (image_picker->clipboard_radio_button, "active", &active, NULL);
  if (active) {
    g_print ("Clipboard active\n");
    ret = image_picker_save_from_clipboard (basename, image_name,
                                            image_path_relative);
  }

out:
  gtk_widget_destroy (GTK_WIDGET (image_picker));
  if (ret == FALSE) {
    if (*image_name != NULL) {
      g_free (*image_name);
      *image_name = NULL;
    }
  }

  return ret;
}
