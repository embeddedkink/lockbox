# lockbox

An open source Emlalock-compatible lock box.

## About

This project aims to create a secure lockbox that can be connected to Emlalock with minimal effort. With a companion app on a desktop, locking up is as easy as running one single command, and uploading the resulting .txt file.
Usage of the box is fully private and the box does not ping out to remote servers.

The project has been designed to be as easy to set up as possible. Off the shelf components like the NodeMCU Amica and an SG90 servo have been chosen for their wide availability. The rest is 3d printed for easy customization.

## Setting up

- Print, or have someone print the box, lid, and locking cam.
- Verify the tolerances of the locking mechanism with special attention for the ridges inside the cam that interface with the servo.
- Aquire an SG90 servo and a NodeMCU Amica (a Lolin does not quite fit the current box design).
- Flash the mcu.
- Connect the servo to the mcu.
- Power the mcu by either:
  - Connecting with a power-only usb micro b cable
  - Connecting with a usb to dupont power cable
- Connect to the box' wifi AP (Lockbox!), and have it connect to your network.
- With the box still open, do a test run.
  - Lock the box.
  - Open the box with the password in the newly created txt file.

## Usage

Connect power up the microcontroller. Store the key in the box inside of a plastic enclosure. Lock the box using the controller, send the password away, and delete it permanently on you local machine. The box can now be turned off, and turned on again when you have the password. For details on how to use the controller, read [the readme](lockbox_control/README.md)

## TODO

- [ ] Verify controller functionality on Windows
- [ ] Add web-based controller
- [ ] Allow addressing box based on user-provided ip
- [ ] Create a key compartment in the box
