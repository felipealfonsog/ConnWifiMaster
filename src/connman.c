#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Define function prototypes
void load_saved_networks(GtkTreeStore *store);
void prompt_for_password_and_connect(const gchar *network_name);
void update_auto_connect_configuration(const gchar *network_name);

// Helper function to detect the WiFi manager
static const char* detect_wifi_manager() {
    FILE *fp;
    char buffer[BUFFER_SIZE];
    const char *wifi_manager = NULL;

    // Check ConnMan
    fp = popen("systemctl is-active connman", "r");
    if (fp != NULL) {
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "active") != NULL) {
                wifi_manager = "connman";
            }
        }
        pclose(fp);
    }

    // Check NetworkManager
    if (wifi_manager == NULL) {
        fp = popen("systemctl is-active NetworkManager", "r");
        if (fp != NULL) {
            if (fgets(buffer, sizeof(buffer), fp) != NULL) {
                if (strstr(buffer, "active") != NULL) {
                    wifi_manager = "NetworkManager";
                }
            }
            pclose(fp);
        }
    }

    return wifi_manager;
}

void load_saved_networks(GtkTreeStore *store) {
    const char *wifi_manager = detect_wifi_manager();
    FILE *fp;
    char buffer[BUFFER_SIZE];
    GtkTreeIter iter;

    if (wifi_manager == NULL) {
        // Fallback to macOS WiFi scan
        fp = popen("system_profiler SPAirPortDataType | grep 'Network Name:'", "r");
        if (fp == NULL) {
            perror("popen");
            return;
        }

        while (fgets(buffer, sizeof(buffer), fp)) {
            GtkTreeIter iter;
            char *network_name = strtok(buffer, ": ");
            if (network_name != NULL) {
                gtk_tree_store_append(store, &iter, NULL);
                gtk_tree_store_set(store, &iter, 0, network_name, 1, FALSE, -1);
            }
        }
        pclose(fp);
    } else if (strcmp(wifi_manager, "connman") == 0) {
        // ConnMan
        fp = popen("connmanctl services", "r");
        if (fp == NULL) {
            perror("popen");
            return;
        }

        while (fgets(buffer, sizeof(buffer), fp)) {
            if (strstr(buffer, "Wifi")) {
                char *network_name = strtok(buffer, " \n");
                if (network_name != NULL) {
                    gtk_tree_store_append(store, &iter, NULL);
                    gtk_tree_store_set(store, &iter, 0, network_name, 1, FALSE, -1);
                }
            }
        }
        pclose(fp);
    } else if (strcmp(wifi_manager, "NetworkManager") == 0) {
        // NetworkManager
        fp = popen("nmcli -f SSID dev wifi", "r");
        if (fp == NULL) {
            perror("popen");
            return;
        }

        while (fgets(buffer, sizeof(buffer), fp)) {
            if (strstr(buffer, "SSID")) continue; // Skip header line
            char *network_name = strtok(buffer, " \n");
            if (network_name != NULL) {
                gtk_tree_store_append(store, &iter, NULL);
                gtk_tree_store_set(store, &iter, 0, network_name, 1, FALSE, -1);
            }
        }
        pclose(fp);
    } else {
        g_print("No supported WiFi manager found.\n");
    }
}

void prompt_for_password_and_connect(const gchar *network_name) {
    GtkWidget *dialog;
    GtkWidget *password_entry;
    GtkWidget *content_area;
    GtkResponseType result;

    dialog = gtk_dialog_new_with_buttons("Enter Password", NULL, GTK_DIALOG_MODAL, 
                                         "Cancel", GTK_RESPONSE_CANCEL, 
                                         "Connect", GTK_RESPONSE_ACCEPT, 
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(content_area), password_entry, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        const gchar *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
        // Connect using connmanctl or nmcli with password
        char command[BUFFER_SIZE];
        if (strstr(network_name, "NetworkManager") != NULL) {
            snprintf(command, sizeof(command), "nmcli dev wifi connect '%s' password '%s'", network_name, password);
        } else {
            snprintf(command, sizeof(command), "connmanctl connect %s %s", network_name, password);
        }
        system(command);
    }

    gtk_widget_destroy(dialog);
}

void update_auto_connect_configuration(const gchar *network_name) {
    // Placeholder for actual auto-connect configuration
    char command[BUFFER_SIZE];
    if (strstr(network_name, "NetworkManager") != NULL) {
        snprintf(command, sizeof(command), "nmcli connection modify '%s' connection.autoconnect yes", network_name);
    } else {
        snprintf(command, sizeof(command), "connmanctl enable %s", network_name);
    }
    system(command);
}
