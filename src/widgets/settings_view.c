#include <gtk/gtk.h>

#include "settings.h"
#include "utils.h"
#include "widgets/settings_view.h"

struct _SettingsView
{
  GtkBox parent_instance;

  /* widgets */
  GtkSwitch *dark_mode_toggle;
  GtkListBoxRow *about_button;

  /* properties */
  Entry *entry;
};

G_DEFINE_TYPE (SettingsView, settings_view, GTK_TYPE_BOX);

static gboolean
dark_mode_toggle_clicked(GtkButton *button, gboolean enabled, gpointer user_data)
{
  settings_set_dark_mode (enabled);
  utils_apply_dark_mode ();
  return FALSE;
}

gboolean
about_button_pressed (GtkWidget *widget, gpointer user_data)
{
  GdkPixbuf *icon_pixbuf;
  GdkTexture *icon_texture;
  GError *err = NULL;
  gchar *authors[2] = { "Johan Bjäreholt", NULL };

  icon_pixbuf = gdk_pixbuf_new_from_resource ("/com/bjareholt/johan/simple-diary/icons/logo.svg", &err);
  if (icon_pixbuf == NULL) {
    g_printerr ("failed to load image: %s\n", err->message);
    g_clear_error (&err);
    return TRUE;
  }

  icon_texture = gdk_texture_new_for_pixbuf (icon_pixbuf);

  gtk_show_about_dialog (NULL,
                         "program-name", "Simple Diary",
                         "title", "About",
                         "logo", icon_texture,
                         "authors", &authors,
                         "license-type", GTK_LICENSE_GPL_3_0,
                         "website", "https://github.com/johan-bjareholt/simple-diary-gtk",
                         NULL);
  return TRUE;
}

static void
settings_view_class_init (SettingsViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
      "/com/bjareholt/johan/simple-diary/ui/settings.ui");
  gtk_widget_class_bind_template_child (widget_class, SettingsView,
      dark_mode_toggle);
  gtk_widget_class_bind_template_child (widget_class, SettingsView,
      about_button);
}

static void
settings_view_init (SettingsView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  g_object_set (self->dark_mode_toggle, "state", settings_get_dark_mode (), NULL);
  g_signal_connect (self->dark_mode_toggle, "state-set",
      (GCallback) dark_mode_toggle_clicked, self);
  g_signal_connect (self->about_button, "activated", (GCallback) about_button_pressed, self);
}

GtkWidget *
settings_view_new ()
{
    SettingsView *settings_view = g_object_new (DIARY_TYPE_SETTINGS_VIEW, NULL);

    return GTK_WIDGET (settings_view);
}
