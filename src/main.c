#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "connman.h"

// Function prototypes
static void on_connect_button_clicked(GtkButton *button, GtkTreeView *treeview);
static void on_auto_connect_toggled(GtkCellRendererToggle *renderer, gchar *path, GtkTreeView *treeview);
static GtkWidget* create_main_window();

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

static void on_auto_connect_toggled(GtkCellRendererToggle *renderer, gchar *path, GtkTreeView *treeview) {
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreeIter iter;
    GtkTreePath *tree_path = gtk_tree_path_new_from_string(path);
    gtk_tree_model_get_iter(model, &iter, tree_path);
    gboolean auto_connect;
    gtk_tree_model_get(model, &iter, 1, &auto_connect, -1);
    auto_connect = !auto_connect;

    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 1, auto_connect, -1);

    gtk_tree_path_free(tree_path);

    // Retrieve network name for auto-connect configuration
    gchar *network_name;
    gtk_tree_model_get(model, &iter, 0, &network_name, -1);

    // Update auto-connect configuration
    if (auto_connect) {
        update_auto_connect_configuration(network_name);
    }

    g_free(network_name);
}

static GtkWidget* create_main_window() {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ConnWifiMaster");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *treeview = gtk_tree_view_new();
    GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Network", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    GtkCellRenderer *toggle_renderer = gtk_cell_renderer_toggle_new();
    g_object_set(toggle_renderer, "activatable", TRUE, NULL);
    GtkTreeViewColumn *toggle_column = gtk_tree_view_column_new_with_attributes("Auto-Connect", toggle_renderer, "active", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), toggle_column);

    g_signal_connect(toggle_renderer, "toggled", G_CALLBACK(on_auto_connect_toggled), treeview);

    gtk_box_pack_start(GTK_BOX(vbox), treeview, TRUE, TRUE, 0);

    GtkWidget *connect_button = gtk_button_new_with_label("Connect");
    gtk_box_pack_start(GTK_BOX(vbox), connect_button, FALSE, FALSE, 0);

    g_signal_connect(connect_button, "clicked", G_CALLBACK(on_connect_button_clicked), treeview);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    return window;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = create_main_window();
    gtk_widget_show_all(window);

    // Load saved networks
    GtkTreeStore *store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(window)));
    load_saved_networks(store);

    gtk_main();
    return 0;
}
