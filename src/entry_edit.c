#include "entry_edit.h"
#include "utils.h"

struct _EntryEdit
{
  GtkBox parent_instance;

  /* widgets */
  GtkTextView *text_view;
  GtkButton *add_image_button;

  /* properties */
  Entry *entry;
};

G_DEFINE_TYPE (EntryEdit, entry_edit, GTK_TYPE_BOX);

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

  buffer = gtk_text_view_get_buffer (self->text_view);
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

  buffer = gtk_text_view_get_buffer (self->text_view);
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  if (strlen (text) == 0)
    goto cleanup;

  if (!entry_write(self->entry, text, &err)) {
    g_error ("failed to save entry: %s\n", err->message);
    g_clear_error (&err);
    goto cleanup;
  }

cleanup:
  g_free (text);
}

static void
entry_edit_finalize (GObject *object)
{
  //EntryEdit *self = DIARY_ENTRY_EDIT (object);

  G_OBJECT_CLASS (entry_edit_parent_class)->finalize (object);
}

static void
entry_edit_constructed (GObject *object)
{
  EntryEdit *self = DIARY_ENTRY_EDIT (object);
  entry_edit_read (self);

  G_OBJECT_CLASS (entry_edit_parent_class)->constructed (object);
}

static gboolean
add_image_button_clicked(GtkButton *button, gpointer user_data)
{
  g_print ("Add image\n");
  return TRUE;
}

static void
entry_edit_class_init (EntryEditClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = entry_edit_finalize;
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
      "/com/johan-bjareholt/simple-diary/ui/entry_edit.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryEdit, text_view);
  gtk_widget_class_bind_template_child (widget_class, EntryEdit,
      add_image_button);
}

static void
entry_edit_init (EntryEdit *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  g_signal_connect (self, "destroy", (GCallback) entry_edit_save, NULL);
  g_signal_connect (self->add_image_button, "clicked",
      (GCallback) add_image_button_clicked, self);
}

GtkWidget *
entry_edit_new (Entry *entry)
{
    EntryEdit *entry_edit = g_object_new (DIARY_TYPE_ENTRY_EDIT, "entry", entry, NULL);

    return GTK_WIDGET (entry_edit);
}
