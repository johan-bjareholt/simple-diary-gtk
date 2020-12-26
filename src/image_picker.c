#include "image_picker.h"
#include "utils.h"

struct _ImagePicker
{
  GtkDialog parent_instance;

  /* widgets */
  GtkButton *add_button;
  GtkButton *cancel_button;
  GtkEntry *name_entry;
  GtkFileChooserButton *file_button;
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
  gtk_widget_class_bind_template_child (widget_class, ImagePicker, file_button);
}

static void
image_picker_init (ImagePicker *self)
{
  //GtkDialog *dialog = GTK_DIALOG (self);

  gtk_widget_init_template (GTK_WIDGET (self));
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

gboolean
image_picker_run (gchar *basename, gchar **image_name, gchar **image_path)
{
  gint result;
  ImagePicker *image_picker;
  gchar *src_path;
  gchar *target_dir;
  gchar *image_extension;
  GError *err = NULL;

  g_assert (image_name != NULL);
  g_assert (image_path != NULL);

  image_picker = g_object_new (DIARY_TYPE_IMAGE_PICKER, NULL);

  result = gtk_dialog_run (GTK_DIALOG (image_picker));

  *image_name = NULL;
  *image_path = NULL;

  switch (result) {
    case GTK_RESPONSE_OK:
      break;
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_NONE:
    case GTK_RESPONSE_DELETE_EVENT:
      goto error;
    default:
      g_print ("Invalid GTK_RESPONSE enum: %d\n", result);
      g_assert_not_reached ();
      goto error;
  }

  src_path = gtk_file_chooser_get_filename (
      GTK_FILE_CHOOSER (image_picker->file_button));
  if (src_path == NULL) {
    /* TODO: display warning to user */
    g_print ("No file chosen, ignoring\n");
    goto error;
  }
  target_dir = utils_get_photos_folder (basename);
  image_extension = utils_get_file_extension (src_path);
  *image_name = g_strdup (gtk_entry_get_text (image_picker->name_entry));
  *image_path = g_strdup_printf ("%s/%s%s", target_dir, *image_name,
      image_extension);
  g_free (target_dir);

  if (!copy_file (src_path, *image_path, &err)) {
    g_print ("Failed to copy image: %s\n", err->message);
    goto error;
  }

  gtk_widget_destroy (GTK_WIDGET (image_picker));

  return TRUE;

error:
  if (*image_name != NULL) {
    g_free (*image_name);
    *image_name = NULL;
  }
  if (*image_path != NULL) {
    g_free (*image_path);
    *image_path = NULL;
  }

  return FALSE;
}
