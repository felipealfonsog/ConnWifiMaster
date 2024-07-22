#include <gtk/gtk.h>
#include "connman.h"

static void on_connect_clicked(GtkButton *button, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeIter iter;
    gchar *network_name;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &network_name, -1);
        connect_to_network(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))), network_name);
        g_free(network_name);
    }
}

static void on_disconnect_clicked(GtkButton *button, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeIter iter;
    gchar *network_name;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &network_name, -1);
        disconnect_from_network(network_name);
        g_free(network_name);
    }
}

static void on_autoconnect_toggled(GtkToggleButton *toggle_button, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeIter iter;
    gchar *network_name;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &network_name, -1);
        set_autoconnect(network_name, gtk_toggle_button_get_active(toggle_button));
        g_free(network_name);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ConnWifiMaster");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *tree_view = gtk_tree_view_new();
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Network", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    column = gtk_tree_view_column_new_with_attributes("Signal", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    column = gtk_tree_view_column_new_with_attributes("Protected", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    gtk_box_pack_start(GTK_BOX(vbox), tree_view, TRUE, TRUE, 0);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *connect_button = gtk_button_new_with_label("Connect");
    g_signal_connect(connect_button, "clicked", G_CALLBACK(on_connect_clicked), tree_view);
    gtk_box_pack_start(GTK_BOX(hbox), connect_button, TRUE, TRUE, 0);

    GtkWidget *disconnect_button = gtk_button_new_with_label("Disconnect");
    g_signal_connect(disconnect_button, "clicked", G_CALLBACK(on_disconnect_clicked), tree_view);
    gtk_box_pack_start(GTK_BOX(hbox), disconnect_button, TRUE, TRUE, 0);

    GtkWidget *autoconnect_button = gtk_check_button_new_with_label("Auto Connect");
    g_signal_connect(autoconnect_button, "toggled", G_CALLBACK(on_autoconnect_toggled), tree_view);
    gtk_box_pack_start(GTK_BOX(hbox), autoconnect_button, TRUE, TRUE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    load_wifi_networks(store);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
