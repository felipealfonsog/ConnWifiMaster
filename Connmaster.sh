#!/bin/bash

CONFIG_FILE="$HOME/.connman_networks"

# Function to scan and list WiFi networks
function scan_networks() {
    echo "Scanning WiFi networks..."
    connmanctl scan wifi
    connmanctl services
}

# Function to get the network ID from the SSID
function get_network_id() {
    ssid=$1
    network_id=$(connmanctl services | grep "$ssid" | awk '{print $NF}')
    echo "$network_id"
}

# Function to check if the network is already configured
function is_network_configured() {
    network_id=$1
    grep -q "$network_id" "$CONFIG_FILE"
    return $?
}

# Function to get the password for a configured network
function get_stored_password() {
    network_id=$1
    grep "$network_id" "$CONFIG_FILE" | awk -F, '{print $2}'
}

# Function to connect to a WiFi network
function connect_network() {
    echo "Enter the WiFi network name (SSID):"
    read ssid

    network_id=$(get_network_id "$ssid")
    
    if [ -z "$network_id" ]; then
        echo "Network not found."
        return
    fi

    # Check if the network is already configured
    if is_network_configured "$network_id"; then
        stored_password=$(get_stored_password "$network_id")
        echo "Network already configured. Using stored password."
        echo "Connecting to $network_id..."
        connmanctl connect "$network_id"
        echo "Connected now"
    else
        echo "Enter the password (leave blank if none):"
        read -s password

        # Start ConnMan agent to handle password entry
        echo "Starting ConnMan agent..."
        connmanctl agent on

        # Attempt to connect to the network
        echo "Connecting to $network_id..."
        connmanctl connect "$network_id"

        # Wait for the user to input the passphrase interactively
        echo "Please input the passphrase manually if prompted by ConnMan."

        # Verify if the connection was successful
        if [ $? -ne 0 ]; then
            echo "Failed to connect to $ssid"
            return
        fi

        if [ -n "$password" ]; then
            echo "Setting passphrase..."
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
    fi
}

# Function to disconnect from a WiFi network
function disconnect_network() {
    echo "Enter the name of the WiFi network (SSID) you want to disconnect:"
    read ssid

    network_id=$(get_network_id "$ssid")

    if [ -z "$network_id" ]; then
        echo "Network not found."
        return
    fi

    connmanctl disconnect "$network_id"
}

# Function to configure autoconnect
function configure_autoconnect() {
    echo "Enter the WiFi network name (SSID):"
    read ssid

    network_id=$(get_network_id "$ssid")

    if [ -z "$network_id" ]; then
        echo "Network not found."
        return
    fi

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

    network_id=$(get_network_id "$ssid")

    if [ -z "$network_id" ]; then
        echo "Network not found."
        return
    fi

    connmanctl config "$network_id" --autoconnect no

    # Update the configuration in the file
    sed -i "/^$network_id,/d" "$CONFIG_FILE"
    echo "$network_id,," >> "$CONFIG_FILE"
}

# Function to display credits
function display_credits() {
    echo "----------------------------------------"
    echo "ConnWifiMaster - ConnMan Network Manager"
    echo "----------------------------------------"
    echo "By Computer Science Engineer: "
    echo "Felipe Alfonso González"
    echo "f.alfonso@res-ear.ch - github.com/felipealfonsog"
    echo "----------------------------------------"
}

# Function to list currently connected networks
function list_connected_networks() {
    echo "Currently connected networks:"
    
    # List connected networks
    connmanctl services | awk '
    # Start reading from the lines where a network is listed
    /^\*/ { 
        connected = 1
        next 
    }
    # Print the SSID if we are in a connected network block
    /^   / && connected { 
        print $2
        connected = 0 
    }'
}



# Main menu
while true; do
    echo "----------------------------------------"
    echo "ConnWifiMaster - ConnMan Network Manager"
    echo "----------------------------------------"
    echo "1. Scan WiFi networks"
    echo "2. Connect to a WiFi network"
    echo "3. Disconnect from a WiFi network"
    echo "4. Configure autoconnect for a WiFi network"
    echo "5. Remove autoconnect configuration from a WiFi network"
    echo "6. List connected networks"
    echo "7. Credits"
    echo "8. Exit"
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
            list_connected_networks
            ;;
        7)
            display_credits
            ;;
        8)
            exit 0
            ;;
        *)
            echo "Invalid option. Please try again."
            ;;
    esac
done
