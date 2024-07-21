#ifndef CONNMAN_H
#define CONNMAN_H

#include <gtk/gtk.h>

void load_saved_networks(GtkTreeStore *store);
void prompt_for_password_and_connect(const gchar *network_name);
void update_auto_connect_configuration(const gchar *network_name);

#endif // CONNMAN_H
