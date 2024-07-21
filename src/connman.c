#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Verifica si connmanctl está disponible
static gboolean is_connman_available() {
    return access("/usr/bin/connmanctl", X_OK) == 0;
}

// Verifica si nmcli está disponible
static gboolean is_networkmanager_available() {
    return access("/usr/bin/nmcli", X_OK) == 0;
}

// Verifica si el sistema es macOS
static gboolean is_macos() {
    return access("/usr/sbin/airport", X_OK) == 0;
}

// Carga redes guardadas según el sistema de gestión de redes detectado
void load_saved_networks(GtkTreeStore *store) {
    if (is_connman_available()) {
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
    } else if (is_networkmanager_available()) {
        FILE *fp = popen("nmcli -f SSID dev wifi", "r");
        if (fp == NULL) {
            perror("popen");
            return;
        }

        char buffer[BUFFER_SIZE];
        while (fgets(buffer, sizeof(buffer), fp)) {
            if (buffer[0] != '\n') {
                GtkTreeIter iter;
                gtk_tree_store_append(store, &iter, NULL);
                gtk_tree_store_set(store, &iter, 0, buffer, 1, FALSE, -1);
            }
        }
        pclose(fp);
    } else if (is_macos()) {
        FILE *fp = popen("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -s", "r");
        if (fp == NULL) {
            perror("popen");
            return;
        }

        char buffer[BUFFER_SIZE];
        while (fgets(buffer, sizeof(buffer), fp)) {
            if (strstr(buffer, "SSID")) {
                continue; // Omitir el encabezado
            }
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, buffer, 1, FALSE, -1);
        }
        pclose(fp);
    }
}

// Muestra un diálogo para ingresar la contraseña y conectar
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
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "connmanctl connect %s %s", network_name, password);
        system(command);
    }

    gtk_widget_destroy(dialog);
}

// Actualiza la configuración de auto-conexión
void update_auto_connect_configuration(const gchar *network_name) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "connmanctl enable %s", network_name);
    system(command);
}
