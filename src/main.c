#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define AIRPORT_PATH "/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport"

static GtkWidget *treeview; // Declarar treeview globalmente para que sea accesible

static void update_auto_connect_configuration(const gchar *network_name) {
    // Implementar configuración de auto-conexión según la red.
    printf("Updating auto-connect for network: %s\n", network_name);
}

static void prompt_for_password_and_connect(const gchar *network_name) {
    // Implementar la lógica para pedir la contraseña y conectar a la red.
    printf("Prompting for password and connecting to network: %s\n", network_name);
}

static void on_connect_button_clicked(GtkButton *button, GtkTreeView *treeview) {
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    gboolean has_selection = gtk_tree_selection_get_selected(selection, &model, &iter);

    if (has_selection) {
        gchar *network_name;
        gtk_tree_model_get(model, &iter, 0, &network_name, -1);

        // Prompt for password if needed and connect
        prompt_for_password_and_connect(network_name);

        g_free(network_name);
    }
}

static void on_auto_connect_toggled(GtkCellRendererToggle *renderer, gchar *path, GtkTreeStore *store) {
    GtkTreeIter iter;
    GtkTreePath *tree_path = gtk_tree_path_new_from_string(path);
    GtkTreeModel *model = GTK_TREE_MODEL(store);
    gboolean auto_connect;

    if (gtk_tree_model_get_iter(model, &iter, tree_path)) {
        gtk_tree_model_get(model, &iter, 1, &auto_connect, -1);
        auto_connect = !auto_connect;

        gtk_tree_store_set(store, &iter, 1, auto_connect, -1);

        gtk_tree_path_free(tree_path);

        // Retrieve network name for auto-connect configuration
        gchar *network_name;
        gtk_tree_model_get(model, &iter, 0, &network_name, -1);

        // Update auto-connect configuration
        update_auto_connect_configuration(network_name);

        g_free(network_name);
    }
}

static GtkWidget* create_main_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ConnWifiMaster");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    treeview = gtk_tree_view_new();
    GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Network", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    GtkCellRenderer *toggle_renderer = gtk_cell_renderer_toggle_new();
    g_object_set(toggle_renderer, "activatable", TRUE, NULL);
    GtkTreeViewColumn *toggle_column = gtk_tree_view_column_new_with_attributes("Auto-Connect", toggle_renderer, "active", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), toggle_column);

    g_signal_connect(toggle_renderer, "toggled", G_CALLBACK(on_auto_connect_toggled), store);

    gtk_box_pack_start(GTK_BOX(vbox), treeview, TRUE, TRUE, 0);

    GtkWidget *connect_button = gtk_button_new_with_label("Connect");
    gtk_box_pack_start(GTK_BOX(vbox), connect_button, FALSE, FALSE, 0);

    g_signal_connect(connect_button, "clicked", G_CALLBACK(on_connect_button_clicked), treeview);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    return window;
}

static void scan_wifi_arch(GtkTreeView *treeview) {
    FILE *fp = popen("nmcli -f SSID,IN-USE dev wifi", "r");
    if (fp == NULL) {
        perror("Failed to run nmcli command");
        return;
    }

    char buffer[256];
    GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(treeview));
    GtkTreeIter iter;
    gboolean in_use;

    while (fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
        if (buffer[0] != '\0' && buffer[0] != 'S') { // Skip header line
            char *ssid = strtok(buffer, " \n");
            char *in_use_str = strtok(NULL, " \n");

            in_use = (in_use_str != NULL && strcmp(in_use_str, "[*]") == 0);

            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, ssid, 1, in_use, -1);
        }
    }

    pclose(fp);
}


static void scan_wifi_macos(GtkTreeView *treeview) {
    char buffer[128];
    FILE *fp = popen(AIRPORT_PATH " -s", "r");
    if (fp == NULL) {
        perror("Failed to run airport command");
        return;
    }

    GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(treeview));
    GtkTreeIter iter;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (buffer[0] != '\0' && buffer[0] != 'S') { // Skip header line
            gchar *ssid = strtok(buffer, "\n");
            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, ssid, 1, FALSE, -1);
        }
    }

    pclose(fp);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = create_main_window();
    gtk_widget_show_all(window);

    // Detect and scan WiFi networks based on system
    if (system("connmanctl services > /dev/null 2>&1") == 0) {
        scan_wifi_arch(GTK_TREE_VIEW(treeview)); // Arch Linux
    } else if (system("nmcli -f SSID dev wifi > /dev/null 2>&1") == 0) {
        scan_wifi_arch(GTK_TREE_VIEW(treeview)); // NetworkManager
    } else {
        scan_wifi_macos(GTK_TREE_VIEW(treeview)); // macOS
    }

    gtk_main();
    return 0;
}
