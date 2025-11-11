#!/bin/bash

CONFIG_FILE="$HOME/.connman_networks"

# Function to scan and list WiFi networks
function scan_networks() {
    echo "Scanning WiFi networks..."
    connmanctl scan wifi
    connmanctl services
}

# Function to connect to a WiFi network
function connect_network() {
    echo "Enter the WiFi network name (SSID):"
    read ssid
    echo "Enter the password (leave blank if none):"
    read -s password

    network_id=$(connmanctl services | grep "$ssid" | awk '{print $3}')
    
    if [ -z "$password" ]; then
        connmanctl connect "$network_id"
    else
        connmanctl connect "$network_id"
        connmanctl config "$network_id" --passphrase "$password"
    fi

    echo "Do you want to set the network to autoconnect? (y/n):"
    read autoconnect

    if [ "$autoconnect" = "y" ]; then
        connmanctl config "$network_id" --autoconnect yes
    else
        connmanctl config "$network_id" --autoconnect no
    fi

    # Save the configuration to the file
    echo "$network_id,$password,$autoconnect" >> "$CONFIG_FILE"
}

# Function to disconnect from a WiFi network
function disconnect_network() {
    echo "Enter the name of the WiFi network (SSID) you want to disconnect:"
    read ssid
    network_id=$(connmanctl services | grep "$ssid" | awk '{print $3}')
    connmanctl disconnect "$network_id"
}

# Function to configure autoconnect
function configure_autoconnect() {
    echo "Enter the WiFi network name (SSID):"
    read ssid
    network_id=$(connmanctl services | grep "$ssid" | awk '{print $3}')
    
    echo "Do you want to set the network to autoconnect? (y/n):"
    read autoconnect

    if [ "$autoconnect" = "y" ]; then
        connmanctl config "$network_id" --autoconnect yes
    else
        connmanctl config "$network_id" --autoconnect no
    fi

    # Update the configuration in the file
    sed -i "/^$network_id,/d" "$CONFIG_FILE"
    echo "$network_id,,$autoconnect" >> "$CONFIG_FILE"
}

# Function to remove autoconnect configuration from a specific network
function remove_autoconnect() {
    echo "Enter the WiFi network name (SSID) to remove autoconnect:"
    read ssid
    network_id=$(connmanctl services | grep "$ssid" | awk '{print $3}')
    connmanctl config "$network_id" --autoconnect no

    # Update the configuration in the file
    sed -i "/^$network_id,/d" "$CONFIG_FILE"
    echo "$network_id,," >> "$CONFIG_FILE"
}

# Function to display credits
function display_credits() {
    echo "****************************************"
    echo "ConnWifiMaster - ConnMan Network Manager"
    echo "----------------------------------------"
    echo "By Computer Science Engineer: "
    echo "Felipe Alfonso González"
    echo "f.alfonso@res-ear.ch - github.com/felipealfonsog"
    echo "****************************************"
}

# Create config file if it doesn't exist
if [ ! -f "$CONFIG_FILE" ]; then
    touch "$CONFIG_FILE"
fi

# Main menu
while true; do
    echo "----------------------------------------"
    echo "ConnMan CLI - Network Manager"
    echo "----------------------------------------"
    echo "1. Scan WiFi networks"
    echo "2. Connect to a WiFi network"
    echo "3. Disconnect from a WiFi network"
    echo "4. Configure autoconnect for a WiFi network"
    echo "5. Remove autoconnect configuration from a WiFi network"
    echo "6. Credits"
    echo "7. Exit"
    echo "----------------------------------------"
    read -p "Choose an option: " option

    case $option in
        1)
            scan_networks
            ;;
        2)
            connect_network
            ;;
        3)
            disconnect_network
            ;;
        4)
            configure_autoconnect
            ;;
        5)
            remove_autoconnect
            ;;
        6)
            display_credits
            ;;
        7)
            exit 0
            ;;
        *)
            echo "Invalid option. Please try again."
            ;;
    esac
done
