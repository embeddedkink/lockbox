#! /usr/bin/python3

import argparse
import json
import requests
import secrets
import socket
import string
import time
from PIL import Image, ImageDraw, ImageFont
from sys import platform
from zeroconf import *

default_passwordfile_text = "./latestpassword.txt"
default_passwordfile_image = "./latestpassword.png"
default_password_file_mode = "text"
mdns_type = '_ekilb._tcp.local.'

class ServiceListener(object):
    def __init__(self):
        self.r = Zeroconf()
        self.devices = []

    def remove_service(self, zeroconf, type, name):
        pass

    def add_service(self, zeroconf, type, name):
        info = self.r.get_service_info(type, name)
        if info:
            address = (socket.inet_ntoa(info.addresses[0]))
            port = info.port
            name = info.name
            self.devices.append({"address": address, "port": port, "name": name})

    def update_service(self):
        pass


def send_command(cmd, host):
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
    elif cmd["command"] == "settings":
        response = requests.get(
            host+"/settings"
        )
    
    return json.loads(response.content)


def generate_password():
    alphabet = string.ascii_letters + string.digits
    password = ''.join(secrets.choice(alphabet) for i in range(20))
    return password


def lock(password, host):
    cmd = {}
    cmd["command"] = "lock"
    cmd["password"] = password
    response = send_command(cmd, host)
    if response['result'] == "success":
        return True
    else:
        return False


def unlock(password, host):
    cmd = {}
    cmd["command"] = "unlock"
    cmd["password"] = password
    response = send_command(cmd, host)
    if response['result'] == "success":
        return True
    else:
        return False


def update(host):
    cmd = {}
    cmd["command"] = "update"
    response = send_command(cmd, host)
    if response['result'] == "success":
        return True
    else:
        return False


def get_settings(host):
    cmd = {}
    cmd["command"] = "settings"
    response = send_command(cmd, host)
    return json.dumps(response["data"])


def find_devices(device_name = None):
    r = Zeroconf()
    listener = ServiceListener()
    browser = ServiceBrowser(r, mdns_type, listener=listener)
    for i in range(5):
        if len(listener.devices) > 0:
            if device_name is not None:
                if (any(d["name"] == device_name for d in listener.devices)):
                    break
            else:
                break
        time.sleep(1)
    r.close()
    return listener.devices


def save_password(password, file, mode="text"):
    if mode == "text":
        f = open(file, "w")
        f.write(password)
        f.close()
    elif mode == "image":
        image_height = 128
        image_width = 512
        margins = 32
        img = Image.new('RGB', (image_width, image_height), color = (0, 0, 0))
        d = ImageDraw.Draw(img)

        if platform == "linux" or platform == "linux2":
            font = ImageFont.truetype("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 28)
        elif platform == "win32":
            font = ImageFont.truetype("arial.ttf", 28)
        else:
            raise "Generating images only supported on linux and windows"

        text_width, text_height = font.getsize(password)
        if text_width > image_width - 2*margins:
            raise "Password too long for image"

        d.text((margins,margins), password, fill=(255,255,255), font=font)
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
                        choices=["lock", "unlock", "update", "getsettings"],
                        required=True)
    parser.add_argument('-p', '--password', dest='password')
    parser.add_argument('-f', '--password-file', dest='password_file')
    parser.add_argument('-m', '--password-filemode', dest='password_file_mode')
    parser.add_argument('-d', '--device-name', dest='device_name',
                        help="e.g. 'lockbox_000000._ekilb._tcp.local.'")
    parser.add_argument('--host-override', dest='host_override',
                        help="e.g. 'http://192.168.0.1:5000'")
    args = parser.parse_args()

    # HOST SELECTION

    picked_host = ""
    if args.host_override is None:
        if args.device_name is None:
            devices = find_devices()
            if (len(devices) == 0):
                print("Error: no lockbox available. Exiting.")
                exit(1)
            elif len(devices) == 1:
                d = devices[0]
                print(f"Found one device: {d['name']}")
                picked_host = f'http://{d["address"]}:{d["port"]}'
            elif len(devices > 1):
                print("Error: too many devices. Select a specific one. Exiting.")
                exit(1)
        else:
            devices = find_devices(args.device_name)
            for d in devices:
                if d["name"] == args.device_name:
                    picked_host = f'http://{d["address"]}:{d["port"]}'
            if picked_host == "":
                print("Error: selected device not found. Exiting.")
                exit(1)
        
    else:
        picked_host = args.host_override
    print(f"Picked host {picked_host}")

    # FILE SELECTION

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

    # ACTION SELECTION

    if args.action == "lock":
        if args.password is not None:
            password = args.password
        else:
            password = generate_password()
        print(f'Password: {password}')
        save_password(password, password_file, password_file_mode)
        if lock(password, picked_host):
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

        if unlock(password, picked_host):
            print("Unlocked!")
        else:
            print("Could not unlock")
    elif args.action == "update":
        if update(picked_host):
            print("updated succesfully")
        else:
            print("update failed")
    elif args.action == "getsettings":
        print("Current settings are: " + get_settings(picked_host))


if __name__ == "__main__":
    main()
