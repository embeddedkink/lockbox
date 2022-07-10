import argparse
import json
from PIL import Image, ImageDraw
import requests
import secrets
import socket
import string
import time
from zeroconf import *

default_passwordfile_text = "./latestpassword.txt"
default_passwordfile_image = "./latestpassword.png"
default_password_file_mode = "text"
mdns_type = '_ekilb._tcp.local.'
devices = []
host = ""

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


def send_command(cmd):
    if cmd["command"] == "lock":
        response = requests.post(
            host+"/lock",
            data={"password": cmd["password"]}
        )
    elif cmd["command"] == "unlock":
        response = requests.post(
            host+"/unlock",
            data={"password": cmd["password"]}
        )
    elif cmd["command"] == "update":
        response = requests.post(
            host+"/update"
        )
    elif cmd["command"] == "status":
        response = requests.get(
            host+"/status"
        )
    
    return json.loads(response.content)


def generate_password():
    alphabet = string.ascii_letters + string.digits
    password = ''.join(secrets.choice(alphabet) for i in range(20))
    return password


def lock(password):
    cmd = {}
    cmd["command"] = "lock"
    cmd["password"] = password
    response = send_command(cmd)
    if response['result'] == "success":
        return True
    else:
        return False


def unlock(password):
    cmd = {}
    cmd["command"] = "unlock"
    cmd["password"] = password
    response = send_command(cmd)
    if response['result'] == "success":
        return True
    else:
        return False


def update():
    cmd = {}
    cmd["command"] = "update"
    response = send_command(cmd)
    if response['result'] == "success":
        return True
    else:
        return False


def get_status():
    cmd = {}
    cmd["command"] = "status"
    response = send_command(cmd)
    return response["data"]


def find_devices():
    r = Zeroconf()
    listener = ServiceListener()
    browser = ServiceBrowser(r, mdns_type, listener=listener)
    for i in range(16):
        if len(devices) > 0: #TODO: Does this reliably return many valid lockboxes? 
            break
        time.sleep(0.5)
    r.close()


def save_password(password, file, mode="text"):
    if mode == "text":
        f = open(file, "w")
        f.write(password)
        f.close()
    elif mode == "image":
        img = Image.new('RGB', (256, 50), color = (0, 0, 0))
        d = ImageDraw.Draw(img)
        d.text((20,20), password, fill=(255,255,255))
        img.save(file)
    else:
        raise "Invalid password mode"


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
    parser.add_argument('-m', '--password-filemode', dest='password_file_mode')
    parser.add_argument('-d', '--device-name', dest='device_name',
                        help="e.g. 'lockbox_000000._ekilb._tcp.local.'")
    parser.add_argument('--host-override', dest='host_override',
                        help="e.g. 'http://192.168.0.1:5000'")
    args = parser.parse_args()

    global host

    if args.host_override is None:
        find_devices()
        if (len(devices) == 0):
            print("Error: no lockbox available")
            exit(1)
        elif len(devices) > 1:
            for d in devices:
                if d["name"] == args.device_name:
                    host = f'http://{d["address"]}:{d["port"]}'
        else:
            d = devices[0]
            if args.device_name is not None:
                if d["name"] != args.device_name:
                    print("Error: device name specified but not found")
                    exit(1)
            host = f'http://{d["address"]}:{d["port"]}'
    else:
        host = args.host_override

    if args.password_file_mode is not None:
        password_file_mode = args.password_file_mode
    else:
        password_file_mode = default_password_file_mode

    if args.password_file is not None:
        password_file = args.password_file
    else:
        if password_file_mode == "image":
            password_file = default_passwordfile_image
        elif password_file_mode == "text":
            password_file = default_passwordfile_text
        else:
            raise "Could not pick correct file name"


    if args.action == "lock":
        if args.password is not None:
            password = args.password
        else:
            password = generate_password()
        print(f'Password: {password}')
        save_password(password, password_file, password_file_mode)
        if lock(password):
            print("Locked!")
        else:
            print("Could not lock")
    elif args.action == "unlock":
        if password_file_mode != "text":
            print("Only text mode is supported for unlocking")
        
        if args.password is not None:
            password = args.password
        else:
            try:
                password = retrieve_password(password_file)
            except FileNotFoundError:
                print("Could not unlock: file not found!")
                password = ""

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
