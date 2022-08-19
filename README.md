# lockbox

An open source safe, cheap and Emlalock-compatible lock box.

## About

This project aims to create a secure lockbox that can be connected to online keyholding services with minimal effort. With a companion app on a desktop, locking up is as easy as running one single command, and uploading the resulting .txt or .png file.
Usage of the box is fully private and neither the box nor the controlling software pings out to remote servers. Privacy is a right we take very seriously.

The project has been designed to be as easy to set up as possible. Off the shelf components like the NodeMCU Amica and an SG90 servo have been chosen for their wide availability. The rest is 3d printed for easy customization.

## Selling points

- Secure. No way to break into the box without physically destroying it
- Safe. Is the emergency bigger than the cost of replacing the box? Just force the lid off. (This does destroy the box!)
- Cheap. Only one actuator and no real time clock necessary
- Compatible with all locks based on small physical keys (larger key compatibility coming soon!)
- Based on common off-the-shelf (COTS) and 3d printed parts
- Fully local system, no dependency on the cloud and no telemetry is collected

## A note on updates

The box can be updated manually but will not be seeking for the latest updates automatically. Updating the box is not hard, but requires some technical steps. An update utility is in the works. Until then, the version of the control software that matches the firmware of your lock box will be accessible in this repository.

## Setting up

- Print, or have someone print the box, lid, and locking cam.
- Aquire an SG90 servo and a NodeMCU Amica (a Lolin does not (yet!) fit the current box design).
- Verify the tolerances of the locking mechanism with special attention for the ridges inside the cam that interface with the servo. If this interface is not solid, you can turn to superglue.
- Flash the MCU.
- Connect the servo to the MCU.
- Power the MCU by providing voltage over the MCU's Vin and Gnd. See the Power heading down below.
- Connect to the box' wifi AP (Lockbox!), and have it connect to your network.
- With the box still open, do a test run.
  - Lock the box.
  - Open the box with the password in the newly created txt file.

## Power

The box may draw 0.4 amps at 5 volts with the servo stalled according to real-world measurements. This should be easily handled by most modern USB devices.
Note that some servos may not perform identically to ours, even if they're marketed as 9g SG90.

The box allows dupont cables to come out of the bottom. You can power it with a female USB micro-b to dupont breakout board along with 2 dupont female to female wires; solder your own USB to dupont cable; or even 3d print a USB to dupont adapter.

## Recovering

If you had to abort a session, the electronics can be reset and used in a new box. This can be done by simply connecting the usb port on the microcontroller, listening on the UART (`picocom -b 9600 /dev/ttyUSB0` or with the Arduino IDE) and pressing the RESET button on the microcontroller. It will now send data about its status and the password. Use this password with the control software to unlock like normal.

## Usage

Connect power to the microcontroller. Put the key into the empty partition of the lockbox. Put this lid onto the box. Lock using the controller, send the password away to your platform of choice (Only Emlalock has been verified), and delete it permanently on you local machine. The box can now be safely turned off. It only needs to be turned on again when you have the password. For details on how to use the controller, read [the readme](lockbox_control/README.md)

## Project status

This lockbox is an ongoing project. We're on version three of the box, and the second major version of the protocol. The following changes are planned for this version:

- [ ] Verify controller functionality on Microsoft Windows (Partially complete)
- [ ] ~~Add web-based controller~~ (Scrapped due to technology restraints)
- [X] Create a key compartment in the box
- [X] Perform destructive tests on the box
- [X] Add compatibility with other online keyholding platforms (image mode)

A new version will support Bluetooth, opening the way for control right from your browser, or from a mobile app. This version is still several months away. Pull requests are welcomed!
