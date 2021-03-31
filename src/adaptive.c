#include <gtk/gtk.h>

static void
init(GtkApplication *app)
{
  GtkApplicationWindow *window;

  window = GTK_APPLICATION_WINDOW (g_object_new (GTK_TYPE_WINDOW, "application", app, NULL));
  gtk_widget_show(GTK_WIDGET (window));
}
int
main(int argc, char *argv[])
{
    GtkApplication *app;
    gtk_init (&argc, &argv);

    app = gtk_application_new ("com.bjareholt.johan.SimpleDiary", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (init), app);

    int status = g_application_run (G_APPLICATION (app), argc, argv);

    return status;
}
