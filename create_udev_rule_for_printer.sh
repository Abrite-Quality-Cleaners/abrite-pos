#!/bin/bash

# Create a udev rule for Epson printer (Vendor: 04b8, Product: 0202)
RULE='SUBSYSTEM=="usb", ATTR{idVendor}=="04b8", ATTR{idProduct}=="0202", MODE="0666"'
RULE_FILE="/etc/udev/rules.d/99-usb-libusb.rules"

echo "Creating udev rule..."
echo $RULE | sudo tee $RULE_FILE

echo "Reloading udev rules..."
sudo udevadm control --reload
sudo udevadm trigger

echo "Done. Please unplug and replug your printer for the new permissions to take effect."