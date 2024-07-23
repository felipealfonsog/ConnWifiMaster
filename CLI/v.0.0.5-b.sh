#!/bin/bash

# Configuration file to store networks and passwords
CONFIG_FILE="$HOME/.connman_networks"

# Function to check if ConnMan is running and start it if not
check_connman() {
    if ! pgrep -x "connmand" > /dev/null; then
        echo "ConnMan is not running. Starting ConnMan..."
        sudo systemctl start connman
        if ! pgrep -x "connmand" > /dev/null; then
            echo "Failed to start ConnMan. Exiting..."
            exit 1
        fi
    fi
}

# Function to scan and list WiFi networks
scan_networks() {
    echo "Scanning WiFi networks..."
    connmanctl scan wifi

    local raw_networks
    IFS=$'\n' read -r -d '' -a raw_networks < <(connmanctl services | grep wifi && printf '\0')

    echo "Available networks:"
    > "$CONFIG_FILE" # Clear the configuration file

    for raw_network in "${raw_networks[@]}"; do
        ssid=$(echo "$raw_network" | awk -F' - ' '{print $1}' | xargs) # Extract SSID
        network_id=$(echo "$raw_network" | awk -F' - ' '{print $2}' | xargs) # Extract Network ID
        
        # Ensure SSID and Network ID are not empty
        if [ -z "$ssid" ]; then
            ssid="Unknown SSID"
        fi
        if [ -z "$network_id" ]; then
            network_id="Unknown Network ID"
        fi

        # Print the result formatted
        printf "%-40s - %s\n" "$ssid" "$network_id"

        # Save network ID to configuration file
        echo "$network_id," >> "$CONFIG_FILE"
    done
}

# Function to check if networks have been scanned
check_scanned() {
    if [ ! -s "$CONFIG_FILE" ]; then
        echo "You need to scan networks first to get the network IDs."
        return 1
    else
        return 0
    fi
}

# Function to connect to a WiFi network
connect_network() {
    check_scanned || return

    echo "Enter the network ID of the WiFi network you want to connect to:"
    read -r network_id

    if [ -z "$network_id" ]; then
        echo "Invalid network ID."
        return
    fi

    echo "Enter the password (leave blank if none):"
    read -s password

    if [ -z "$password" ]; then
        connmanctl connect $network_id
    else
        connmanctl tether wifi "$network_id" $password
    fi

    # Check if the operation was successful
    if [ $? -eq 0 ]; then
        echo "Operation successful."
        # Save network ID and password to configuration file
        sed -i "/^$network_id,/ s/,$//" "$CONFIG_FILE"
        echo "$network_id,$password" >> "$CONFIG_FILE"
    else
        echo "Operation failed."
    fi
}

# Function to disconnect from a WiFi network
disconnect_network() {
    check_scanned || return

    echo "Enter the network ID of the WiFi network you want to disconnect from:"
    read -r network_id
    
    if [ -z "$network_id" ]; then
        echo "Invalid network ID."
        return
    fi

    # Print the command being executed
    echo "Executing command: connmanctl disconnect \"$network_id\""

    connmanctl disconnect "$network_id"

    # Check if the operation was successful
    if [ $? -eq 0 ]; then
        echo "Operation successful."
    else
        echo "Operation failed."
    fi
}

# Function to enable or disable autoconnect
configure_autoconnect() {
    check_scanned || return

    echo "Enter the network ID of the WiFi network you want to configure:"
    read -r network_id
    
    if [ -z "$network_id" ]; then
        echo "Invalid network ID."
        return
    fi

    echo "Do you want to set the network to autoconnect? (y/n):"
    read -r autoconnect

    if [ "$autoconnect" = "y" ]; then
        command="connmanctl config $network_id --autoconnect yes"
    else
        command="connmanctl config $network_id --autoconnect no"
    fi

    # Print the command being executed
    echo "Executing command: $command"

    # Execute the command
    eval $command

    # Check if the operation was successful
    if [ $? -eq 0 ]; then
        echo "Operation successful."
    else
        echo "Operation failed."
    fi
}

# Function to connect to a saved network with priority to autoconnect
connect_saved_network() {
    echo "Attempting to connect to saved network..."

    # Check each network ID in the configuration file
    while IFS=, read -r network_id password; do
        if [ -z "$network_id" ]; then
            continue
        fi

        echo "Attempting to connect to saved network: $network_id"
        
        if [ -z "$password" ]; then
            connmanctl connect $network_id
        else
            connmanctl tether wifi $network_id $password
        fi

        # Check if connection was successful
        if [ $? -eq 0 ]; then
            echo "Connected to $network_id"
            return
        else
            echo "Failed to connect to $network_id."
        fi
    done < "$CONFIG_FILE"

    echo "No saved networks available or all attempts failed."
}

# Function to display credits
display_credits() {
    echo "***************************************************"
    echo "ConnWifiMaster CLI - ConnMan Network Manager"
    echo "___________________________________________________"
    echo "By Computer Science Engineer: "
    echo "Felipe Alfonso González"
    echo "f.alfonso@res-ear.ch - github.com/felipealfonsog"
    echo "***************************************************"
}

# Create configuration file if it doesn't exist
if [ ! -f "$CONFIG_FILE" ]; then
    touch "$CONFIG_FILE"
fi

# Check if ConnMan is active
check_connman

# Main menu
while true; do
    echo "---------------------------------------------------"
    echo "ConnWifiMaster CLI - ConnMan Network Manager"
    echo "___________________________________________________"
    echo "1. Scan/List WiFi networks"
    echo "2. Connect to a WiFi network"
    echo "3. Disconnect from a WiFi network"
    echo "4. Enable or Disable autoconnect for a WiFi network"
    echo "5. Connect to a saved Wifi network"
    echo "6. Credits by Engineer"
    echo "7. Exit"
    echo "---------------------------------------------------"
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
            connect_saved_network
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
