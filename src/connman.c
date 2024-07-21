#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

void load_saved_networks(GtkTreeStore *store) {
    FILE *fp = popen("connmanctl services", "r");
    if (fp == NULL) {
        perror("popen");
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strstr(buffer, "Wifi")) {
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, buffer, 1, FALSE, -1);
        }
    }

    pclose(fp);
}

void prompt_for_password_and_connect(const gchar *network_name) {
    GtkWidget *dialog;
    GtkWidget *password_entry;
    GtkWidget *content_area;
    GtkResponseType result;

    dialog = gtk_dialog_new_with_buttons("Enter Password", NULL, GTK_DIALOG_MODAL, 
                                         ("Cancel"), GTK_RESPONSE_CANCEL, 
                                         ("Connect"), GTK_RESPONSE_ACCEPT, 
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(content_area), password_entry, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        const gchar *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
        // Connect using connmanctl with password
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "connmanctl connect %s %s", network_name, password);
        system(command);
    }

    gtk_widget_destroy(dialog);
}

void update_auto_connect_configuration(const gchar *network_name) {
    // Placeholder for actual auto-connect configuration
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "connmanctl enable %s", network_name);
    system(command);
}
