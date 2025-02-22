#include <gtkmdeditor/gtkmdeditor.h>

#include "entry_edit.h"
#include "image_picker.h"
#include "utils.h"
#include "window.h"
#include "headerbuttons.h"

struct _EntryEdit
{
  GtkBox parent_instance;

  /* widgets */
  GtkScrolledWindow *entry_edit_viewport;
  GtkMdEditor *md_editor;
  GtkButton *add_image_button;

  /* properties */
  Entry *entry;
  gboolean unsaved_changes;
};

static void entry_edit_save (EntryEdit *self);

static void
on_back_pressed (GtkWidget *widget)
{
  EntryEdit *self = DIARY_ENTRY_EDIT (widget);
  entry_edit_save (self);
  header_buttons_control_default_on_back_pressed (widget);
}

static void
diary_header_buttons_control_init (HeaderButtonsControlInterface *iface)
{
    iface->on_back_pressed = on_back_pressed;
}

G_DEFINE_TYPE_WITH_CODE (EntryEdit, entry_edit, GTK_TYPE_BOX,
    G_IMPLEMENT_INTERFACE (DIARY_TYPE_HEADER_BUTTONS, diary_header_buttons_control_init));

typedef enum
{
  PROP_ENTRY = 1,
  NUM_PROPS,
} EntryProperties;

static GParamSpec *obj_properties[NUM_PROPS] = { NULL, };

static void
entry_edit_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  EntryEdit *self = DIARY_ENTRY_EDIT (object);

  switch ((EntryProperties) property_id) {
    case PROP_ENTRY:
      self->entry = g_value_get_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
entry_edit_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  EntryEdit *self = DIARY_ENTRY_EDIT (object);

  switch ((EntryProperties) property_id)
    {
    case PROP_ENTRY:
      g_value_set_object (value, self->entry);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void entry_edit_read(EntryEdit *self)
{
  GError *err = NULL;
  GtkTextBuffer *buffer;
  gchar *body;

  body = entry_read (self->entry, &err);
  if (err != NULL) {
    g_warning ("Failed to read entry");
    goto cleanup;
  }

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(self->md_editor));
  gtk_text_buffer_set_text (buffer, body, -1);

cleanup:
  if (body != NULL) {
    g_free (body);
  }
}

static void
entry_edit_save(EntryEdit *self)
{
  GtkTextIter start, end;
  gchar *text;
  GtkTextBuffer *buffer;
  GError *err = NULL;

  if (self->unsaved_changes == FALSE)
    return;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(self->md_editor));
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  if (!entry_write(self->entry, text, &err)) {
    g_error ("failed to save entry: %s\n", err->message);
    g_clear_error (&err);
    goto cleanup;
  }

  self->unsaved_changes = FALSE;

cleanup:
  g_free (text);
}

static void
entry_edit_constructed (GObject *object)
{
  EntryEdit *self = DIARY_ENTRY_EDIT (object);
  entry_edit_read (self);

  G_OBJECT_CLASS (entry_edit_parent_class)->constructed (object);
}

void
image_picker_cb (GObject *obj,
                 GAsyncResult *res,
                 gpointer user_data)
{
    ImagePicker *image_picker = DIARY_IMAGE_PICKER_DIALOG (obj);
    EntryEdit *entry_edit = DIARY_ENTRY_EDIT (user_data);

    g_autoptr(Image) image = image_picker_dialog_open_finish(image_picker, res);
    if (image != NULL) {
        g_print ("Adding image named '%s' from path '%s'\n", image->name, image->path);
        GtkTextBuffer *text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(entry_edit->md_editor));
        g_autofree char *md_image_link = g_strdup_printf ("![%s](<%s>)", image->name, image->path);
        gtk_text_buffer_insert_at_cursor (text_buffer, md_image_link, strlen (md_image_link));
    }
}

static gboolean
add_image_button_clicked(GtkButton *button, gpointer user_data)
{
  EntryEdit *entry_edit = DIARY_ENTRY_EDIT (user_data);
  GtkWindow *window;
  g_autofree gchar *basename;

  window = GTK_WINDOW (diary_window_get_instance ());
  g_object_get (entry_edit->entry, "basename", &basename, NULL);

  ImagePicker *image_picker = image_picker_dialog_new (basename);
  image_picker_dialog_open (image_picker, window, image_picker_cb, entry_edit);

  return TRUE;
}

static void
entry_edit_class_init (EntryEditClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = entry_edit_constructed;

  object_class->get_property = entry_edit_get_property;
  object_class->set_property = entry_edit_set_property;

  obj_properties[PROP_ENTRY] =
    g_param_spec_object ("entry",
                         "entry",
                         "entry",
                         DIARY_TYPE_ENTRY,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     NUM_PROPS,
                                     obj_properties);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/entry_edit.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryEdit, entry_edit_viewport);
  gtk_widget_class_bind_template_child (widget_class, EntryEdit, add_image_button);
}

static void
entry_edit_init (EntryEdit *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  self->unsaved_changes = TRUE;
  g_signal_connect (self, "destroy", (GCallback) entry_edit_save, NULL);
  g_signal_connect (self->add_image_button, "clicked",
      G_CALLBACK (add_image_button_clicked), self);
  self->md_editor = GTK_MD_EDITOR(gtk_md_editor_new());
  g_object_set(self->entry_edit_viewport, "child", self->md_editor, NULL);
}

GtkWidget *
entry_edit_new (Entry *entry)
{
  EntryEdit *entry_edit = g_object_new (DIARY_TYPE_ENTRY_EDIT, "entry", entry, NULL);

  return GTK_WIDGET (entry_edit);
}
