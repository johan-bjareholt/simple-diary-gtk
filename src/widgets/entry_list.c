#include "utils.h"
#include "entry.h"
#include "entry_listing.h"
#include "entry_list.h"

struct _EntryList
{
  GtkScrolledWindow parent_instance;

  GtkWidget *entries;
};

G_DEFINE_TYPE (EntryList, entry_list, GTK_TYPE_SCROLLED_WINDOW);

static gint
sort_ptrarray_alphabetically(gconstpointer a, gconstpointer b)
{
    gchar *a_str = *(gchar **) a;
    gchar *b_str = *(gchar **) b;

    return -g_strcmp0 (a_str, b_str);
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

  g_ptr_array_sort (files, (GCompareFunc) sort_ptrarray_alphabetically);
  g_dir_close (dir);

  return files;
}

static GtkWidget *
generate_entry_list (const gchar *dir_path, GPtrArray *files)
{
  const gchar *filename;
  GtkWidget *entry_list_widget;
  GtkListBox *entry_list_box;

  entry_list_widget = gtk_list_box_new ();
  entry_list_box = GTK_LIST_BOX (entry_list_widget);
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (entry_list_box), GTK_SELECTION_NONE);

  for (int i=0; i < files->len; i++) {
    filename = g_ptr_array_index (files, i);
    Entry *entry = entry_open (dir_path, filename);
    GtkWidget *entry_listing = GTK_WIDGET (entry_listing_new (entry));
    GtkWidget *row_widget = gtk_list_box_row_new ();
    gtk_container_add (GTK_CONTAINER (row_widget), entry_listing);
    gtk_list_box_insert (entry_list_box, row_widget, -1);
  }

  return entry_list_widget;
}

static void
load_entry_list (EntryList *self, gpointer user_data)
{
  gchar *dir_path;
  GPtrArray *files;

  dir_path = utils_get_diary_folder ();
  files = list_entries (dir_path);
  self->entries = generate_entry_list (dir_path, files);
  gtk_container_add (GTK_CONTAINER (self), self->entries);
  gtk_widget_show_all (GTK_WIDGET (self));

  g_free (dir_path);
  g_ptr_array_unref (files);
}

static void
unload_entry_list (EntryList *self, gpointer user_data)
{
  gtk_container_remove (GTK_CONTAINER (self), self->entries);
}

static void
entry_list_class_init (EntryListClass *klass)
{
  //GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
}

static void
entry_list_init (EntryList *self)
{
  self->entries = NULL;
  // We load and unload on ever map/unmap so the entry list is updated in case
  // a entry has been deleted or added
  g_assert (g_signal_connect (self, "map", (GCallback) load_entry_list, NULL) > 0);
  g_assert (g_signal_connect (self, "unmap", (GCallback) unload_entry_list, NULL) > 0);
}

GtkWidget *
entry_list_new (void)
{
    GtkWidget *entry_list = g_object_new (DIARY_TYPE_ENTRY_LIST, NULL);
    return entry_list;
}
