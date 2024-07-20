import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QPushButton, QWidget, QCheckBox, QListWidget, QListWidgetItem, QInputDialog, QMessageBox
import subprocess

class ConnWifiMaster(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('ConnWifiMaster')

        self.networkList = QListWidget()
        self.refreshButton = QPushButton('Refresh Networks')
        self.autoConnectButton = QPushButton('Set Auto-Connect')
        self.connectButton = QPushButton('Connect')
        self.disconnectButton = QPushButton('Disconnect')

        self.refreshButton.clicked.connect(self.refreshNetworks)
        self.autoConnectButton.clicked.connect(self.setAutoConnect)
        self.connectButton.clicked.connect(self.connectNetwork)
        self.disconnectButton.clicked.connect(self.disconnectNetwork)

        layout = QVBoxLayout()
        layout.addWidget(self.networkList)
        layout.addWidget(self.refreshButton)
        layout.addWidget(self.autoConnectButton)
        layout.addWidget(self.connectButton)
        layout.addWidget(self.disconnectButton)

        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)
        self.refreshNetworks()

    def refreshNetworks(self):
        result = subprocess.run(['connmanctl', 'services'], capture_output=True, text=True)
        self.networkList.clear()
        for line in result.stdout.splitlines():
            item = QListWidgetItem(line)
            checkbox = QCheckBox()
            self.networkList.addItem(item)
            self.networkList.setItemWidget(item, checkbox)

    def setAutoConnect(self):
        selectedItems = self.networkList.selectedItems()
        if selectedItems:
            item = selectedItems[0]
            service_id = item.text().split()[0]
            checkbox = self.networkList.itemWidget(item)
            autoconnect = 'yes' if checkbox.isChecked() else 'no'
            subprocess.run(['connmanctl', 'config', service_id, '--autoconnect', autoconnect])

    def connectNetwork(self):
        selectedItems = self.networkList.selectedItems()
        if selectedItems:
            item = selectedItems[0]
            service_id = item.text().split()[0]
            result = subprocess.run(['connmanctl', 'connect', service_id], capture_output=True, text=True)
            if 'Passphrase' in result.stdout:
                password, ok = QInputDialog.getText(self, 'Enter Password', 'Password:')
                if ok:
                    subprocess.run(['connmanctl', 'connect', service_id, '--passphrase', password])
                else:
                    QMessageBox.warning(self, 'Warning', 'Password is required to connect.')
            self.refreshNetworks()

    def disconnectNetwork(self):
        selectedItems = self.networkList.selectedItems()
        if selectedItems:
            item = selectedItems[0]
            service_id = item.text().split()[0]
            subprocess.run(['connmanctl', 'disconnect', service_id])
            self.refreshNetworks()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = ConnWifiMaster()
    ex.show()
    sys.exit(app.exec_())
