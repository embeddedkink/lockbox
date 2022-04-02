# lockbox

An open source Emlalock-compatible lock box.

## About

This project aims to create a secure lockbox that can be connected to Emlalock with minimal effort. With a companion app on a desktop, locking up is as easy as running one single command, and uploading the resulting .txt file.
Usage of the box is fully private and the box does not ping out to remote servers.

The project has been designed to be as easy to set up as possible. Off the shelf components like the NodeMCU Amica and an SG90 servo have been chosen for their wide availability. The rest is 3d printed for easy customization.

## Features

- Secure. No way to break into the box without physically destroying it
- Safe. Is the emergency bigger than the cost of replacing the box? Smash it with a hammer
- Cheap. one actuator and no real time clock hardware necessary
- Compatible with all key-based locks
- Based on common off-the-shelf (COTS) and 3d printed parts
- Fully local system, no dependency on the cloud or telemetry collected

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

## Recovering

If you had to abort a session, the electronics can be reset and used in a new box. This can be done by simply connecting the usb port on the microcontroller, listening on the UART (`picocom -b 9600 /dev/ttyUSB0`) and pressing the RESET button on the microcontroller. It will now send data about its status and the password. Use the control software to unlock like normal.

## Usage

Connect power up the microcontroller. Put the key in a plastic box or bag and into the lockbox. Lock the box using the controller, send the password away, and delete it permanently on you local machine. The box can now be turned off, and turned on again when you have the password. For details on how to use the controller, read [the readme](lockbox_control/README.md)

## TODO

- [ ] Verify controller functionality on Windows
- [ ] Add web-based controller
- [ ] Allow addressing box based on user-provided ip
- [ ] Create a key compartment in the box
- [ ] Perform destructive tests on PLA and PETG versions of box
- [ ] Assess compatibility with other online keyholding platforms
