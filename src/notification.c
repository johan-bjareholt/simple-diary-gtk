#include "notification.h"

void notification_send (GApplication *application, gchar *title, gchar *body) {
    GNotification *notification;

    notification = g_notification_new (title);
    g_notification_set_body (notification, body);
    // TODO: Fix ID for each notification type
    g_application_send_notification (application, "unique-id", notification);
    g_object_unref (notification);
}
