#!/bin/bash

CONFIG_FILE="$HOME/.connman_networks"

# Function to check if ConnMan is active and running
function check_connman() {
    systemctl is-active --quiet connman
    if [ $? -ne 0 ]; then
        echo -n "ConnMan is not active. Do you want to start it? (y/n) [y]: "
        read start_connman
        start_connman=${start_connman:-y}
        if [ "$start_connman" = "y" ]; then
            sudo systemctl start connman
            if [ $? -ne 0 ]; then
                echo "Failed to start ConnMan. Please check your system configuration."
                exit 1
            fi
            echo "ConnMan started successfully."
        else
            echo "ConnMan is required for this script to function. Exiting."
            exit 1
        fi
    fi
}

# Function to connect to an autoconnect network
function connect_autoconnect_network() {
    autoconnect_network_id=$(grep ',yes$' "$CONFIG_FILE" | cut -d ',' -f 1)
    if [ -n "$autoconnect_network_id" ]; then
        echo "Connecting to autoconnect network $autoconnect_network_id..."
        connmanctl connect "$autoconnect_network_id"
        if [ $? -ne 0 ]; then
            echo "Failed to connect to autoconnect network."
        else
            echo "Connected to autoconnect network."
        fi
    else
        echo "No autoconnect network found. Attempting to connect to another saved network..."
        saved_network_id=$(head -n 1 "$CONFIG_FILE" | cut -d ',' -f 1)
        if [ -n "$saved_network_id" ]; then
            echo "Connecting to saved network $saved_network_id..."
            connmanctl connect "$saved_network_id"
            if [ $? -ne 0 ]; then
                echo "Failed to connect to saved network."
            else
                echo "Connected to saved network."
            fi
        else
            echo "No saved networks found."
        fi
    fi
}

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

# Function to connect to a WiFi network
function connect_network() {
    echo "Enter the WiFi network name (SSID):"
    read ssid

    network_id=$(get_network_id "$ssid")
    
    if [ -z "$network_id" ]; then
        echo "Network not found."
        return
    fi

    # Check if network is already configured with a password
    if grep -q "^$network_id," "$CONFIG_FILE"; then
        password=$(grep "^$network_id," "$CONFIG_FILE" | cut -d ',' -f 2)
        autoconnect=$(grep "^$network_id," "$CONFIG_FILE" | cut -d ',' -f 3)
        echo "Connecting to $network_id..."
        connmanctl connect "$network_id"
        echo "Connected to $ssid"
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

# Function to list currently connected networks
function list_connected_networks() {
    echo "Currently connected networks:"
    connmanctl services | grep "^\*AO" | awk '{print $3}'
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

# Create config file if it doesn't exist
if [ ! -f "$CONFIG_FILE" ]; then
    touch "$CONFIG_FILE"
fi

# Check if ConnMan is active
check_connman

# Main menu
while true; do
    echo "----------------------------------------"
    echo "ConnWifiMaster - ConnMan Network Manager"
    echo "----------------------------------------"
    echo "1. Scan WiFi networks"
    echo "2. Connect to a WiFi network"
    echo "3. Connect to autoconnect or next saved network"
    echo "4. Disconnect from a WiFi network"
    echo "5. Configure autoconnect for a WiFi network"
    echo "6. Remove autoconnect configuration from a WiFi network"
    echo "7. List connected networks"
    echo "8. Credits"
    echo "9. Exit"
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
            connect_autoconnect_network
            ;;
        4)
            disconnect_network
            ;;
        5)
            configure_autoconnect
            ;;
        6)
            remove_autoconnect
            ;;
        7)
            list_connected_networks
            ;;
        8)
            display_credits
            ;;
        9)
            exit 0
            ;;
        *)
            echo "Invalid option. Please try again."
            ;;
    esac
done
