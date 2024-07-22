import sys
import subprocess
from PyQt5.QtWidgets import QApplication, QMainWindow, QTableWidget, QTableWidgetItem, QVBoxLayout, QWidget, QPushButton, QCheckBox

class WiFiManager(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("ConnWifiMaster")
        self.setGeometry(100, 100, 600, 400)
        
        # Main widget and layout
        main_widget = QWidget()
        layout = QVBoxLayout()

        # Table for WiFi networks
        self.table = QTableWidget()
        self.table.setColumnCount(3)  # Updated column count
        self.table.setHorizontalHeaderLabels(["SSID", "AutoConnect", "Status"])
        layout.addWidget(self.table)

        # Set column width for SSID
        self.table.setColumnWidth(0, 300)  # Adjust the width as needed
        self.table.setColumnWidth(1, 100)  # Width for AutoConnect
        self.table.setColumnWidth(2, 100)  # Width for Status

        # Buttons
        self.refresh_button = QPushButton("Refresh")
        self.refresh_button.clicked.connect(self.load_wifi_networks)
        layout.addWidget(self.refresh_button)

        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_wifi)
        layout.addWidget(self.connect_button)

        self.disconnect_button = QPushButton("Disconnect")
        self.disconnect_button.clicked.connect(self.disconnect_wifi)
        layout.addWidget(self.disconnect_button)

        # Setup main widget
        main_widget.setLayout(layout)
        self.setCentralWidget(main_widget)
        
        # Initial load of WiFi networks
        self.load_wifi_networks()

    def load_wifi_networks(self):
        networks = self.get_wifi_networks()
        self.table.setRowCount(len(networks))
        for i, network in enumerate(networks):
            self.table.setItem(i, 0, QTableWidgetItem(network['ssid']))
            
            auto_connect_cb = QCheckBox()
            auto_connect_cb.setChecked(network['autoconnect'])
            self.table.setCellWidget(i, 1, auto_connect_cb)
            
            # Set connection status
            status_item = QTableWidgetItem(network['status'])
            self.table.setItem(i, 2, status_item)

    def get_wifi_networks(self):
        """Get WiFi networks using connmanctl."""
        networks = []
        
        # Trigger a scan
        subprocess.run(['connmanctl', 'scan', 'wifi'], capture_output=True, text=True)
        
        # Get the list of available services
        result = subprocess.run(['connmanctl', 'services'], capture_output=True, text=True)
        lines = result.stdout.splitlines()

        # Get a list of currently connected services
        connected_services = self.get_connected_services()

        for line in lines:
            if line.startswith('  *') or line.startswith('  '):
                parts = line.split()
                if len(parts) > 1:
                    ssid = parts[0].strip('*').strip()
                    service_id = parts[1]
                    
                    # Determine if autoconnect is enabled
                    autoconnect = self.is_autoconnect(service_id)
                    
                    # Determine the connection status
                    status = 'Connected' if service_id in connected_services else 'Disconnected'
                    
                    networks.append({
                        'ssid': ssid,
                        'autoconnect': autoconnect,
                        'status': status
                    })

        return networks

    def get_connected_services(self):
        """Get a list of currently connected services."""
        result = subprocess.run(['connmanctl', 'services'], capture_output=True, text=True)
        lines = result.stdout.splitlines()
        connected_services = set()
        for line in lines:
            if '*' in line:
                parts = line.split()
                if len(parts) > 1:
                    service_id = parts[1]
                    connected_services.add(service_id)
        return connected_services

    def is_autoconnect(self, service_id):
        """Determine if a service is set to autoconnect."""
        result = subprocess.run(['connmanctl', 'config', service_id], capture_output=True, text=True)
        output = result.stdout
        return 'AutoConnect=true' in output

    def connect_wifi(self):
        row = self.table.currentRow()
        if row >= 0:
            ssid = self.table.item(row, 0).text()
            
            # Map SSID to the service ID
            service_id = self.get_service_id_from_ssid(ssid)
            
            if service_id:
                try:
                    subprocess.run(['connmanctl', 'connect', service_id], check=True)
                    self.load_wifi_networks()  # Refresh the network list
                except subprocess.CalledProcessError as e:
                    print(f"Error connecting to {ssid}: {e}")
            else:
                print(f"Service ID not found for SSID: {ssid}")

    def disconnect_wifi(self):
        row = self.table.currentRow()
        if row >= 0:
            ssid = self.table.item(row, 0).text()
            
            # Map SSID to the service ID
            service_id = self.get_service_id_from_ssid(ssid)
            
            if service_id:
                try:
                    subprocess.run(['connmanctl', 'disconnect', service_id], check=True)
                    self.load_wifi_networks()  # Refresh the network list
                except subprocess.CalledProcessError as e:
                    print(f"Error disconnecting from {ssid}: {e}")
            else:
                print(f"Service ID not found for SSID: {ssid}")

    def get_service_id_from_ssid(self, ssid):
        """Get the service ID for a given SSID."""
        result = subprocess.run(['connmanctl', 'services'], capture_output=True, text=True)
        lines = result.stdout.splitlines()
        for line in lines:
            if line.startswith('  *') or line.startswith('  '):
                parts = line.split()
                if len(parts) > 1:
                    service_ssid = parts[0].strip('*').strip()
                    service_id = parts[1]
                    if service_ssid == ssid:
                        return service_id
        return None

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle('Fusion')  # Set the style to Fusion or another available style
    window = WiFiManager()
    window.show()
    sys.exit(app.exec_())
