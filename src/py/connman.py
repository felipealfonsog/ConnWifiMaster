import subprocess

def check_connman():
    try:
        subprocess.check_call(['pgrep', '-x', 'connmand'])
    except subprocess.CalledProcessError:
        print("ConnMan is not running. Starting ConnMan...")
        subprocess.check_call(['sudo', 'systemctl', 'start', 'connman'])
        subprocess.check_call(['pgrep', '-x', 'connmand'])

def scan_networks():
    subprocess.check_call(['connmanctl', 'scan', 'wifi'])
    result = subprocess.check_output(['connmanctl', 'services']).decode('utf-8')
    networks = [line for line in result.split('\n') if 'wifi' in line]
    return networks

def connect_network(network_id, password=''):
    if password:
        command = ['connmanctl', 'tether', 'wifi', network_id, password]
    else:
        command = ['connmanctl', 'connect', network_id]
    subprocess.check_call(command)

def disconnect_network(network_id):
    subprocess.check_call(['connmanctl', 'disconnect', network_id])

def configure_autoconnect(network_id, autoconnect):
    if not network_id:
        raise ValueError("Network ID cannot be empty.")
    command = ['connmanctl', 'config', network_id, f'--autoconnect={autoconnect}']
    subprocess.check_call(command)

def connect_saved_networks(config_file):
    with open(config_file, 'r') as file:
        for line in file:
            network_id, password = line.strip().split(',')
            if password:
                connect_network(network_id, password)
            else:
                connect_network(network_id)
