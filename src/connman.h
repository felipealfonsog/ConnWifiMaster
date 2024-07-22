#ifndef CONNMAN_H
#define CONNMAN_H

#include <gtk/gtk.h>

void load_wifi_networks(GtkListStore *store);
void connect_to_network(GtkWindow *parent, const gchar *service);
void disconnect_from_network(const gchar *service);
void set_autoconnect(const gchar *service, gboolean autoconnect);

#endif // CONNMAN_H
