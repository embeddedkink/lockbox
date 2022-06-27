import socket
import json
import time
import secrets
import string
import sys
from zeroconf import *
import argparse

default_passwordfile = "./latestpassword.txt"
mdns_type = '_ekilb._tcp.local.'
devices = []
device = ()


class ServiceListener(object):
    def __init__(self):
        self.r = Zeroconf()

    def remove_service(self, zeroconf, type, name):
        pass

    def add_service(self, zeroconf, type, name):
        info = self.r.get_service_info(type, name)
        if info:
            address = (socket.inet_ntoa(info.addresses[0]))
            port = info.port
            name = info.name
            devices.append({"address": address, "port": port, "name": name})

    def update_service(self):
        pass


def send_command(command):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(device)
    command += "\n"
    sock.sendall(bytes(command, 'utf-8'))
    data = sock.recv(512)
    sock.close()
    return json.loads(data.decode('utf-8').strip())


def generate_password():
    alphabet = string.ascii_letters + string.digits
    password = ''.join(secrets.choice(alphabet) for i in range(20))
    return password


def lock(password):
    cmd = {}
    cmd["command"] = "lock"
    cmd["password"] = password
    response = send_command(json.dumps(cmd))
    if response['result'] == "success":
        return True
    else:
        return False


def unlock(password):
    cmd = {}
    cmd["command"] = "unlock"
    cmd["password"] = password
    response = send_command(json.dumps(cmd))
    if response['result'] == "success":
        return True
    else:
        return False


def update():
    cmd = {}
    cmd["command"] = "update"
    response = send_command(json.dumps(cmd))
    if response['result'] == "success":
        return True
    else:
        return False


def get_status():
    cmd = {}
    cmd["command"] = "status"
    response = send_command(json.dumps(cmd))
    return response["status"]


def find_devices():
    r = Zeroconf()
    listener = ServiceListener()
    browser = ServiceBrowser(r, mdns_type, listener=listener)
    for i in range(16):
        if len(devices) > 0:
            break
        time.sleep(0.5)
    r.close()


def save_password(password, file):
    f = open(file, "w")
    f.write(password)
    f.close()


def retrieve_password(file):
    f = open(file, "r")
    password = f.readline().strip()
    f.close()
    return password


def main():
    parser = argparse.ArgumentParser(description='Control the EKI Lockbox')
    parser.add_argument('-a', '--action', dest='action',
                        choices=["lock", "unlock", "update", "getstatus"])
    parser.add_argument('-p', '--password', dest='password')
    parser.add_argument('-f', '--password-file', dest='password_file')
    parser.add_argument('-d', '--device-name', dest='device_name',
                        help="e.g. 'lockbox_000000._ekilb._tcp.local.'")
    args = parser.parse_args()

    global device
    find_devices()
    if (len(devices) < 0):
        print("Error: no device in reach")
        exit(1)
    elif len(devices) > 1:
        for d in devices:
            if d["name"] == args.device_name:
                device = (d["address"], d["port"])
    else:
        d = devices[0]
        if args.device_name is not None:
            if d["name"] != args.device_name:
                print("Error: device name specified but not found")
                exit(1)
        device = (d["address"], d["port"])

    if args.password_file is not None:
        password_file = args.password_file
    else:
        password_file = default_passwordfile

    if args.action == "lock":
        password = generate_password()
        print(password)
        save_password(password, password_file)
        if lock(password):
            print("Locked!")
        else:
            print("Could not lock")
    elif args.action == "unlock":
        password = args.password
        if password == None:
            password = retrieve_password(password_file)
        if unlock(password):
            print("Unlocked!")
        else:
            print("Could not unlock")
    elif args.action == "update":
        if update():
            print("updated succesfully")
        else:
            print("update failed")
    else:
        print("Current status is: " + get_status())


if __name__ == "__main__":
    main()
