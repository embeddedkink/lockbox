# lockbox

## About

This project aims to create a secure lockbox whose password can be easily sent somehwere else for safekeeping.

## Setting up

- Print, or have someone print the box, lid, and locking cam.
- Verify the tolerances of the locking mechanism, this is not something you want to have fail.
- Flash the mcu.
- Connect the servo.
- Connect to the box' wifi AP, and have it connect to your network.
- With the box still open, do a test run.
  - Connect to the right IP address.
  - Lock the box.
  - Open the box with the password in the newly created txt file.

## Usage

Connect the mcu with a power-only usb cable. Lock the box, send the password away, and delete it permanently, on you local machine. The box can now be turned off, and turned on again when you have the password.

The password and locked state are stored in emulated EEPROM, on flash.

## TODO

- [ ] Fix undefined behaviour after reflashing (locked state is saved in flash)
- [ ] Make the controller autodiscover the box.
