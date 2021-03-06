# lockbox

An open source safe, cheap and Emlalock-compatible lock box.

## About

This project aims to create a secure lockbox that can be connected to online keyholding services with minimal effort. With a companion app on a desktop, locking up is as easy as running one single command, and uploading the resulting .txt file.
Usage of the box is fully private and neither the box nor the controlling software pings out to remote servers. Privacy is a right we take very seriously.

The project has been designed to be as easy to set up as possible. Off the shelf components like the NodeMCU Amica and an SG90 servo have been chosen for their wide availability. The rest is 3d printed for easy customization.

## Selling points

- Secure. No way to break into the box without physically destroying it
- Safe. Is the emergency bigger than the cost of replacing the box? Just pull the lid off. (This does destroy the box! A more safe design for less abled bodied people is coming soon.)
- Cheap. Only one actuator and no real time clock necessary
- Compatible with all locks based on small physical keys (larger key compatibility coming soon!)
- Based on common off-the-shelf (COTS) and 3d printed parts
- Fully local system, no dependency on the cloud or telemetry collected

## Setting up

- Print, or have someone print the box, lid, and locking cam.
- Aquire an SG90 servo and a NodeMCU Amica (a Lolin does not (yet!) fit the current box design).
- Verify the tolerances of the locking mechanism with special attention for the ridges inside the cam that interface with the servo. If this interface is not solid, you can turn to superglue.
- Flash the MCU.
- Connect the servo to the MCU.
- Power the MCU by providing voltage over the MCU's Vin and Gnd. (The box allows dupont cables to come out of the bottom)
- Connect to the box' wifi AP (Lockbox!), and have it connect to your network.
- With the box still open, do a test run.
  - Lock the box.
  - Open the box with the password in the newly created txt file.

## Recovering

If you had to abort a session, the electronics can be reset and used in a new box. This can be done by simply connecting the usb port on the microcontroller, listening on the UART (`picocom -b 9600 /dev/ttyUSB0` or with the Arduino IDE) and pressing the RESET button on the microcontroller. It will now send data about its status and the password. Use this password with the control software to unlock like normal.

## Usage

Connect power to the microcontroller. Put the key into the empty partition of the lockbox. Put this lid onto the box. Lock using the controller, send the password away to your platform of choice (Only Emlalock has been verified), and delete it permanently on you local machine. The box can now be safely turned off. It only needs to be turned on again when you have the password. For details on how to use the controller, read [the readme](lockbox_control/README.md)

## TODO

- [ ] Verify controller functionality on Microsoft Windows
- [ ] Add web-based controller
- [X] Create a key compartment in the box
- [X] Perform destructive tests on the box
- [ ] Assess compatibility with other online keyholding platforms
