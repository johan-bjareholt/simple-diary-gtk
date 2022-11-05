#include "utils.h"
#include "entry.h"
#include "entry_listing.h"
#include "entry_list.h"

struct _EntryList
{
  GtkWindow parent_instance;

  GtkListBox *entry_list_box;
};

enum
{
  SIGNAL_SELECTION_CHANGED,
  LAST_SIGNAL,
};

static guint entry_list_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EntryList, entry_list, GTK_TYPE_SCROLLED_WINDOW);

static gint
list_box_sort (GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer user_data)
{
  EntryListing *listing1;
  EntryListing *listing2;
  Entry *entry1;
  Entry *entry2;
  gchar *name1;
  gchar *name2;
  gint ret;

  listing1 = DIARY_ENTRY_LISTING (gtk_list_box_row_get_child (GTK_LIST_BOX_ROW (row1)));
  listing2 = DIARY_ENTRY_LISTING (gtk_list_box_row_get_child (GTK_LIST_BOX_ROW (row2)));

  entry1 = entry_listing_get_entry (listing1);
  entry2 = entry_listing_get_entry (listing2);

  g_object_get (entry1, "filename", &name1, NULL);
  g_object_get (entry2, "filename", &name2, NULL);

  ret = -g_strcmp0 (name1, name2);

  g_free (name1);
  g_free (name2);

  return ret;
}

static GPtrArray *
list_entries (const gchar * dir_path)
{
  GDir *dir;
  GError *error;
  const gchar *filename;
  GPtrArray *files;
  files = g_ptr_array_new_full(50, g_free);

  dir = g_dir_open(dir_path, 0, &error);
  while ((filename = g_dir_read_name(dir))) {
    gchar *file_path = g_strdup_printf ("%s/%s", dir_path, filename);
    if (g_file_test (file_path, G_FILE_TEST_IS_REGULAR) &&
        g_str_has_suffix (file_path, ".md")) {
      g_ptr_array_add (files, g_strdup (filename));
    }
    g_free (file_path);
  }

  g_dir_close (dir);

  return files;
}

static void
entry_pressed_cb (GtkListBox *entry_list_box, GtkListBoxRow *row, gpointer user_data)
{
  /* necessary for making no entry selected by default */
  gtk_list_box_set_selection_mode (entry_list_box, GTK_SELECTION_SINGLE);
  gtk_list_box_select_row (entry_list_box, row);
}

static void
entry_selected_changed_cb (GtkListBox *entry_list_box, gpointer user_data)
{
  EntryList *list = DIARY_ENTRY_LIST (user_data);
  GtkListBoxRow *row;
  Entry *entry = NULL;

  row = gtk_list_box_get_selected_row (GTK_LIST_BOX (entry_list_box));
  if (row) {
    gchar *name;
    GtkWidget *child;
    EntryListing *listing;

    child = gtk_list_box_row_get_child (row);
    listing = DIARY_ENTRY_LISTING (child);
    entry = entry_listing_get_entry (listing);
    g_object_get (entry, "basename", &name, NULL);
    g_print ("%s\n", name);
    g_free (name);
  } else {
    g_print ("entry unselected\n");
  }

  g_signal_emit_by_name (list, "selection-changed", entry);
}

void
entry_list_add_entry (EntryList *self, Entry *entry)
{
  GtkWidget *entry_listing = GTK_WIDGET (entry_listing_new (entry));
  GtkWidget *row_widget = gtk_list_box_row_new ();
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row_widget), entry_listing);
  gtk_list_box_insert (self->entry_list_box, row_widget, -1);
}

static GtkListBox *
generate_entry_list (EntryList *self, const gchar *dir_path, GPtrArray *files)
{
  const gchar *filename;

  gtk_list_box_set_selection_mode (self->entry_list_box, GTK_SELECTION_NONE);

  g_signal_connect (self->entry_list_box, "row-activated", G_CALLBACK (entry_pressed_cb), NULL);
  g_signal_connect (self->entry_list_box, "selected-rows-changed", G_CALLBACK (entry_selected_changed_cb), self);

  for (int i=0; i < files->len; i++) {
    filename = g_ptr_array_index (files, i);
    Entry *entry = entry_new (dir_path, filename);
    entry_list_add_entry (self, entry);
  }

  return self->entry_list_box;
}

static void
load_entry_list (EntryList *self)
{
  gchar *dir_path;
  GPtrArray *files;

  dir_path = utils_get_diary_folder ();
  files = list_entries (dir_path);
  generate_entry_list (self, dir_path, files);

  g_free (dir_path);
  g_ptr_array_unref (files);
}

static void
entry_list_class_init (EntryListClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/bjareholt/johan/simple-diary/ui/entry_list.ui");
  gtk_widget_class_bind_template_child (widget_class, EntryList, entry_list_box);

  entry_list_signals [SIGNAL_SELECTION_CHANGED] =
      g_signal_new ("selection-changed", G_TYPE_FROM_CLASS (klass),
      0, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, DIARY_TYPE_ENTRY);
}

typedef struct {
  gboolean found;
  gchar *filename;
} EntryFindCtx;

static void
is_entry (GtkListBoxRow *row, EntryFindCtx *ctx)
{
  Entry *entry;
  gchar *filename;
  EntryListing *listing;
  if (ctx->found)
    return;

  listing = DIARY_ENTRY_LISTING (gtk_list_box_row_get_child (row));

  entry = entry_listing_get_entry (listing);
  g_object_get (entry, "filename", &filename, NULL);
  if (g_strcmp0 (filename, ctx->filename) == 0) {
    ctx->found = TRUE;
  }
  g_free (filename);
}

GtkListBoxRow *
entry_list_find (EntryList *self, gchar *filename)
{
  EntryFindCtx ctx;
  ctx.found = FALSE;
  ctx.filename = filename;
  GtkListBoxRow *row = NULL;

  /* TODO: Fix ugly loop */
  gint i = 0;
  do {
    row = gtk_list_box_get_row_at_index (self->entry_list_box, i);
    if (row == NULL) {
      break;
    }
    is_entry (GTK_LIST_BOX_ROW (row), &ctx);
    if (ctx.found) {
      break;
    }
    i++;
  } while (TRUE);

  return row;
}

void
entry_list_unfocus (EntryList *self)
{
  gtk_list_box_select_row (self->entry_list_box, NULL);
}

static void
entry_list_init (EntryList *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  load_entry_list (self);

  gtk_list_box_set_sort_func (self->entry_list_box, list_box_sort, NULL, NULL);

  gtk_widget_set_vexpand (GTK_WIDGET (self), TRUE);
  gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
}

GtkWidget *
entry_list_new (void)
{
    GtkWidget *entry_list = g_object_new (DIARY_TYPE_ENTRY_LIST, NULL);
    return entry_list;
}

gboolean
entry_list_remove (EntryList *self, Entry *entry)
{
  gboolean ret = FALSE;
  GtkListBoxRow *row;
  gchar *filename;

  g_object_get (entry, "filename", &filename, NULL);

  row = entry_list_find (self, filename);
  if (!row) {
    goto no_match;
  }

  gtk_list_box_remove (self->entry_list_box, GTK_WIDGET (row));

  ret = TRUE;

no_match:
  g_free (filename);
  return ret;
}

void
entry_list_focus (EntryList *self, GtkListBoxRow *row)
{
  GtkWidget *child;
  EntryListing *listing;
  Entry *entry;

  gtk_list_box_select_row (self->entry_list_box, GTK_LIST_BOX_ROW (row));

  child = gtk_list_box_row_get_child (row);
  listing = DIARY_ENTRY_LISTING (child);
  entry = entry_listing_get_entry (listing);
  g_signal_emit_by_name (self, "selection-changed", entry);
}
