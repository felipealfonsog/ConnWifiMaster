import sys
import subprocess
from PyQt5.QtWidgets import (QApplication, QWidget, QVBoxLayout, QPushButton, QMessageBox, 
                             QInputDialog, QTextEdit, QLineEdit)
from PyQt5.QtGui import QClipboard
from PyQt5.QtCore import Qt

class ConnWifiMasterApp(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()
    
    def init_ui(self):
        self.setWindowTitle('ConnWifiMaster GUI')
        self.setGeometry(100, 100, 600, 400)
        
        layout = QVBoxLayout()
        
        # Scan Networks Button
        self.scan_button = QPushButton('Scan/List WiFi networks')
        self.scan_button.clicked.connect(self.scan_networks)
        layout.addWidget(self.scan_button)
        
        # Connect Button
        self.connect_button = QPushButton('Connect to a WiFi network')
        self.connect_button.clicked.connect(self.connect_network)
        layout.addWidget(self.connect_button)
        
        # Disconnect Button
        self.disconnect_button = QPushButton('Disconnect from a WiFi network')
        self.disconnect_button.clicked.connect(self.disconnect_network)
        layout.addWidget(self.disconnect_button)
        
        # Autoconnect Button
        self.autoconnect_button = QPushButton('Enable/Disable autoconnect')
        self.autoconnect_button.clicked.connect(self.configure_autoconnect)
        layout.addWidget(self.autoconnect_button)
        
        # Connect Saved Network Button
        self.connect_saved_button = QPushButton('Connect to a saved WiFi network')
        self.connect_saved_button.clicked.connect(self.connect_saved_network)
        layout.addWidget(self.connect_saved_button)
        
        # Credits Button
        self.credits_button = QPushButton('Credits')
        self.credits_button.clicked.connect(self.display_credits)
        layout.addWidget(self.credits_button)
        
        # Exit Button
        self.exit_button = QPushButton('Exit')
        self.exit_button.clicked.connect(self.close)
        layout.addWidget(self.exit_button)
        
        # Text Edit for displaying network information
        self.text_edit = QTextEdit()
        self.text_edit.setReadOnly(True)
        layout.addWidget(self.text_edit)
        
        # Button to copy selected network ID to clipboard
        self.copy_button = QPushButton('Copy Network ID to Clipboard')
        self.copy_button.clicked.connect(self.copy_to_clipboard)
        layout.addWidget(self.copy_button)
        
        self.setLayout(layout)
    
    def run_command(self, command):
        try:
            result = subprocess.run(command, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            return result.stdout
        except subprocess.CalledProcessError as e:
            return f"Error: {e.stderr}"
    
    def scan_networks(self):
        command = "connmanctl scan wifi && connmanctl services"
        result = self.run_command(command)
        self.text_edit.setPlainText(result)
    
    def connect_network(self):
        network_id, ok = QInputDialog.getText(self, 'Connect to Network', 'Enter Network ID:')
        if not ok or not network_id:
            return
        
        # Check if already connected
        status = self.run_command(f"connmanctl services | grep '{network_id}'")
        if "Connected" in status:
            QMessageBox.information(self, 'Connect to Network', 'Already connected to this network.')
            return
        
        # Try to connect without a password
        result = self.run_command(f"connmanctl connect {network_id}")
        
        if "Password" in result or "failed" in result.lower():
            # Password required
            password, ok = QInputDialog.getText(self, 'Connect to Network', 'Enter Password (leave blank if none):', QLineEdit.Password)
            if not ok:
                return
            if password:
                result = self.run_command(f"connmanctl tether wifi {network_id} {password}")
            else:
                result = self.run_command(f"connmanctl connect {network_id}")
        
        # Check if the operation was successful
        if "success" in result.lower() or "connected" in result.lower():
            QMessageBox.information(self, 'Connect to Network', 'Operation successful.')
        else:
            QMessageBox.warning(self, 'Connect to Network', 'Operation not successful. ' + result)
    
    def disconnect_network(self):
        network_id, ok = QInputDialog.getText(self, 'Disconnect from Network', 'Enter Network ID:')
        if not ok or not network_id:
            return
        command = f"connmanctl disconnect {network_id}"
        result = self.run_command(command)
        if "success" in result.lower():
            QMessageBox.information(self, 'Disconnect from Network', 'Operation successful.')
        else:
            QMessageBox.warning(self, 'Disconnect from Network', 'Operation not successful. ' + result)
    
    def configure_autoconnect(self):
        network_id, ok = QInputDialog.getText(self, 'Configure Autoconnect', 'Enter Network ID:')
        if not ok or not network_id:
            return
        
        # Check the current autoconnect status
        status_command = f"connmanctl services"
        status_result = self.run_command(status_command)
        
        if network_id not in status_result:
            QMessageBox.warning(self, 'Configure Autoconnect', f'Network ID {network_id} not found.')
            return
        
        # Extract current autoconnect status
        current_status = 'no'
        lines = status_result.split('\n')
        for line in lines:
            if network_id in line:
                if 'AutoConnect=true' in line:
                    current_status = 'yes'
                break

        # Prompt for new autoconnect status
        autoconnect, ok = QInputDialog.getItem(self, 'Configure Autoconnect', 'Set Autoconnect:', ['yes', 'no'], current_status == 'yes', editable=False)
        if not ok:
            return
        
        # Convert 'yes'/'no' to 'true'/'false'
        autoconnect_value = 'true' if autoconnect == 'yes' else 'false'
        
        # Run the command to configure autoconnect
        command = f"connmanctl config {network_id} --autoconnect {autoconnect_value}"
        result = self.run_command(command)
        
        # Check if the operation was successful
        if "success" in result.lower() or "configured" in result.lower():
            QMessageBox.information(self, 'Configure Autoconnect', 'Operation successful.')
        else:
            QMessageBox.warning(self, 'Configure Autoconnect', f'Operation not successful. Command result: {result}')

    def connect_saved_network(self):
        command = "while IFS=, read -r network_id password; do if [ -z \"$network_id\" ]; then continue; fi; connmanctl connect \"$network_id\" || connmanctl tether wifi \"$network_id\" \"$password\"; done < ~/.connman_networks"
        result = self.run_command(command)
        if "success" in result.lower():
            QMessageBox.information(self, 'Connect to Saved Network', 'Operation successful.')
        else:
            QMessageBox.warning(self, 'Connect to Saved Network', 'Operation not successful. ' + result)
    
    def copy_to_clipboard(self):
        selected_text = self.text_edit.textCursor().selectedText()
        if selected_text:
            clipboard = QApplication.clipboard()
            clipboard.setText(selected_text)
            QMessageBox.information(self, 'Copied to Clipboard', 'Network ID copied to clipboard.')
        else:
            QMessageBox.warning(self, 'No Selection', 'No text selected to copy.')

    def display_credits(self):
        credits = (
            "***************************************************\n"
            "ConnWifiMaster GUI - ConnMan Network Manager\n"
            "___________________________________________________\n"
            "By Computer Science Engineer:\n"
            "Felipe Alfonso González\n"
            "f.alfonso@res-ear.ch - github.com/felipealfonsog\n"
            "***************************************************"
        )
        QMessageBox.information(self, 'Credits', credits)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    
    # Set application style
    app.setStyle('Fusion')
    
    window = ConnWifiMasterApp()
    window.show()
    sys.exit(app.exec_())
