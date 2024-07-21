#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Function prototypes
static gboolean execute_command(const char *command, char *output, size_t output_size);
static void scan_with_connman(GtkTreeStore *store);
static void scan_with_networkmanager(GtkTreeStore *store);
static void scan_with_mac_wifi(GtkTreeStore *store);

void load_saved_networks(GtkTreeStore *store) {
    // Try ConnMan first
    if (access("/usr/bin/connmanctl", X_OK) == 0) {
        scan_with_connman(store);
    }
    // Try NetworkManager if ConnMan is not available
    else if (access("/usr/bin/nmcli", X_OK) == 0) {
        scan_with_networkmanager(store);
    }
    // Fallback to macOS WiFi scan
    else {
        scan_with_mac_wifi(store);
    }
}

static gboolean execute_command(const char *command, char *output, size_t output_size) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return FALSE;
    }

    size_t len = fread(output, 1, output_size - 1, fp);
    if (len == (size_t)-1) {
        perror("fread");
        pclose(fp);
        return FALSE;
    }

    output[len] = '\0'; // Null-terminate the output
    pclose(fp);
    return TRUE;
}

static void scan_with_connman(GtkTreeStore *store) {
    char buffer[BUFFER_SIZE];
    if (execute_command("connmanctl services", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            if (strstr(line, "Wifi")) {
                GtkTreeIter iter;
                gtk_tree_store_append(store, &iter, NULL);
                gtk_tree_store_set(store, &iter, 0, line, 1, FALSE, -1);
            }
            line = strtok(NULL, "\n");
        }
    }
}

static void scan_with_networkmanager(GtkTreeStore *store) {
    char buffer[BUFFER_SIZE];
    if (execute_command("nmcli -f SSID,ACTIVE dev wifi", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            if (strstr(line, "SSID")) { // Skip header
                line = strtok(NULL, "\n");
                continue;
            }
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, line, 1, FALSE, -1);
            line = strtok(NULL, "\n");
        }
    }
}

static void scan_with_mac_wifi(GtkTreeStore *store) {
    char buffer[BUFFER_SIZE];
    // Try using airport first
    if (execute_command("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -s", buffer, sizeof(buffer))) {
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            if (strstr(line, "SSID")) { // Skip header
                line = strtok(NULL, "\n");
                continue;
            }
            GtkTreeIter iter;
            gtk_tree_store_append(store, &iter, NULL);
            gtk_tree_store_set(store, &iter, 0, line, 1, FALSE, -1);
            line = strtok(NULL, "\n");
        }
    } else {
        // Try using wdutil if airport is deprecated or not working
        if (execute_command("wdutil -list", buffer, sizeof(buffer))) {
            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                GtkTreeIter iter;
                gtk_tree_store_append(store, &iter, NULL);
                gtk_tree_store_set(store, &iter, 0, line, 1, FALSE, -1);
                line = strtok(NULL, "\n");
            }
        }
    }
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
