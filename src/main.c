#include <gtk/gtk.h>
#include "connman.h"

static GtkWidget *text_view;
static GtkTextBuffer *text_buffer;

static void append_text(const char *text) {
    if (text != NULL) {
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(text_buffer, &end);
        gtk_text_buffer_insert(text_buffer, &end, text, -1);
    } else {
        gtk_text_buffer_set_text(text_buffer, "Error: No text to display.\n", -1);
    }
}

static void scan_networks(GtkWidget *widget, gpointer data) {
    char *output = run_command("connmanctl scan wifi && connmanctl services");
    if (output != NULL) {
        gtk_text_buffer_set_text(text_buffer, output, -1);
        free(output);
    } else {
        gtk_text_buffer_set_text(text_buffer, "Error: Failed to scan networks.\n", -1);
    }
}

static void connect_network(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    const char *network_id;
    
    dialog = gtk_dialog_new_with_buttons("Connect to Network",
                                         GTK_WINDOW(data),
                                         GTK_DIALOG_MODAL,
                                         "_OK", GTK_RESPONSE_OK,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        network_id = gtk_entry_get_text(GTK_ENTRY(entry));
        char command[256];
        snprintf(command, sizeof(command), "connmanctl connect %s", network_id);
        char *output = run_command(command);
        append_text(output);
        free(output);
    }
    gtk_widget_destroy(dialog);
}

static void disconnect_network(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    const char *network_id;
    
    dialog = gtk_dialog_new_with_buttons("Disconnect from Network",
                                         GTK_WINDOW(data),
                                         GTK_DIALOG_MODAL,
                                         "_OK", GTK_RESPONSE_OK,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        network_id = gtk_entry_get_text(GTK_ENTRY(entry));
        char command[256];
        snprintf(command, sizeof(command), "connmanctl disconnect %s", network_id);
        char *output = run_command(command);
        append_text(output);
        free(output);
    }
    gtk_widget_destroy(dialog);
}

static void configure_autoconnect(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    const char *network_id;
    GtkWidget *checkbox;
    gboolean autoconnect;

    dialog = gtk_dialog_new_with_buttons("Configure Autoconnect",
                                         GTK_WINDOW(data),
                                         GTK_DIALOG_MODAL,
                                         "_OK", GTK_RESPONSE_OK,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         NULL);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    checkbox = gtk_check_button_new_with_label("Enable Autoconnect");
    gtk_container_add(GTK_CONTAINER(content_area), checkbox);
    
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        network_id = gtk_entry_get_text(GTK_ENTRY(entry));
        autoconnect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox));
        char command[256];
        snprintf(command, sizeof(command), "connmanctl config %s --autoconnect %s", network_id, autoconnect ? "yes" : "no");
        char *output = run_command(command);
        append_text(output);
        free(output);
    }
    gtk_widget_destroy(dialog);
}

static void connect_saved_network(GtkWidget *widget, gpointer data) {
    char *output = run_command("while IFS=, read -r network_id password; do if [ -z \"$network_id\" ]; then continue; fi; connmanctl connect \"$network_id\" || connmanctl tether wifi \"$network_id\" \"$password\"; done < ~/.connman_networks");
    append_text(output);
    free(output);
}

static void copy_to_clipboard(GtkWidget *widget, gpointer data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(text_buffer, &start, &end);
    char *text = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);
    gtk_clipboard_set_text(clipboard, text, -1);
    g_free(text);
}

static void display_credits(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(data),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    "***************************************************\n"
                                    "ConnWifiMaster GUI - ConnMan Network Manager\n"
                                    "___________________________________________________\n"
                                    "By Computer Science Engineer:\n"
                                    "Felipe Alfonso González\n"
                                    "f.alfonso@res-ear.ch - github.com/felipealfonsog\n"
                                    "***************************************************");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *button;
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "ConnWifiMaster GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    button = gtk_button_new_with_label("Scan/List WiFi networks");
    g_signal_connect(button, "clicked", G_CALLBACK(scan_networks), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Connect to a WiFi network");
    g_signal_connect(button, "clicked", G_CALLBACK(connect_network), window);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Disconnect from a WiFi network");
    g_signal_connect(button, "clicked", G_CALLBACK(disconnect_network), window);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Enable/Disable autoconnect");
    g_signal_connect(button, "clicked", G_CALLBACK(configure_autoconnect), window);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Connect to a saved WiFi network");
    g_signal_connect(button, "clicked", G_CALLBACK(connect_saved_network), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Copy Network ID to Clipboard");
    g_signal_connect(button, "clicked", G_CALLBACK(copy_to_clipboard), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Credits");
    g_signal_connect(button, "clicked", G_CALLBACK(display_credits), window);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Exit");
    g_signal_connect(button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    
    gtk_main();
    
    return 0;
}
