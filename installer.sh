#!/bin/bash

welcome() {
    echo "
    ╔═══════════════════════════════════════╗
    ║                                       ║
    ║   ~ ConnWifiMaster App ~              ║
    ║   Developed with ❤️ by                 ║
    ║   Felipe Alfonso González L.          ║
    ║   Computer Science Engineer           ║
    ║   Chile                               ║
    ║                                       ║
    ║   Contact: f.alfonso@res-ear.ch       ║
    ║   Licensed under BSD 3-clause         ║
    ║   GitHub: github.com/felipealfonsog   ║
    ║                                       ║
    ╚═══════════════════════════════════════╝
    "
    echo "Welcome to the ConnWifiMaster App - Bash installer!"
    echo "---------------------------------------------------"
}

install_dependencies() {
    echo "Installing dependencies..."
    sudo apt-get update 
    sudo apt-get install -y git dialog wireless-tools
}

configure_wifi() {
    echo "Configuring Wi-Fi..."
    sudo dialog --title "Wi-Fi Configuration" \
    --inputbox "Enter your Wi-Fi SSID:" 0 0 2> /


    SSID=$(cat /tmp/wifi_ssid.txt)
    sudo dialog --title "Wi-Fi Configuration" \
    --inputbox "Enter your Wi-Fi password:" 0 0 2> /tmp
    PASSWORD=$(cat /tmp/wifi_password.txt)
    sudo dialog --title "Wi-Fi Configuration" \
    --msgbox "Configuring Wi-Fi: SSID: $SSID, Password: $PASSWORD
    sudo wifi-conf $SSID <<< "$PASSWORD\n$PASSWORD"
}

install_connwifimaster() {
    echo "Installing ConnWifiMaster..."
    git clone https://github.com/felipealfonsog/connwifimaster
    cd connwifimaster
    sudo make install
}

welcome
install_dependencies
configure_wifi
install_connwifimaster

echo "ConnWifiMaster App has been successfully installed!"
echo "You can now use the 'connwifimaster' command to manage your Wi-Fi connections."
echo "Enjoy!"

