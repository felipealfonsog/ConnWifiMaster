#include "connman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void execute_command(const gchar *command, gchar *output, size_t size) {
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        g_printerr("Failed to run command: %s\n", command);
        return;
    }
    if (output) {
        if (fgets(output, size, pipe) == NULL) {
            g_printerr("Failed to read command output\n");
        } else {
            g_print("Command output: %s", output);
        }
    }
    if (pclose(pipe) == -1) {
        g_printerr("Failed to close command pipe\n");
    }
}

void load_wifi_networks(GtkListStore *store) {
    execute_command("connmanctl scan wifi", NULL, 0);

    FILE *pipe = popen("connmanctl services", "r");
    if (!pipe) {
        g_printerr("Failed to run command\n");
        return;
    }

    gchar buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        gchar *line = g_strstrip(buffer);
        if (strstr(line, "wifi") != NULL) {
            gchar *service = strtok(line, " ");
            gchar *signal = strtok(NULL, " ");
            gchar *protected = strtok(NULL, " ");

            if (service && signal && protected) {
                gchar *is_protected = (strstr(protected, "protected") != NULL) ? "TRUE" : "FALSE";
                
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                   0, service, // Network name
                                   1, g_strconcat(signal, " dBm", NULL), // Signal strength
                                   2, is_protected, // Protected
                                   -1);
            }
        }
    }
    pclose(pipe);
}

void connect_to_network(GtkWindow *parent, const gchar *service) {
    gchar command[256];
    g_snprintf(command, sizeof(command), "connmanctl connect %s", service);
    gchar output[1024];
    execute_command(command, output, sizeof(output));

    if (strstr(output, "Passphrase") != NULL) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Enter Password",
                                                        parent,
                                                        GTK_DIALOG_MODAL,
                                                        "_OK",
                                                        GTK_RESPONSE_OK,
                                                        "_Cancel",
                                                        GTK_RESPONSE_CANCEL,
                                                        NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE); // Hide password
        gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 0);
        gtk_widget_show_all(dialog);

        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_OK) {
            const gchar *password = gtk_entry_get_text(GTK_ENTRY(entry));
            gchar command_with_password[256];
            g_snprintf(command_with_password, sizeof(command_with_password),
                       "connmanctl config %s --passphrase=%s", service, password);
            execute_command(command_with_password, NULL, 0);
        }

        gtk_widget_destroy(dialog);
    }
}

void disconnect_from_network(const gchar *service) {
    gchar command[256];
    g_snprintf(command, sizeof(command), "connmanctl disconnect %s", service);
    execute_command(command, NULL, 0);
}

void set_autoconnect(const gchar *service, gboolean autoconnect) {
    gchar command[256];
    g_snprintf(command, sizeof(command), "connmanctl config %s --autoconnect=%s", service, autoconnect ? "true" : "false");
    execute_command(command, NULL, 0);
}
