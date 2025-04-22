#!/bin/bash
# Find your device’s bus-port and interface (e.g., 1-2:1.0)
lsusb     # note VID:PID (e.g., 0C45:8009)
ls /sys/bus/usb/devices/1-2:1.0/driver  # should show usbhid before unbind

# Unbind from usbhid
echo -n "1-2:1.0" | sudo tee /sys/bus/usb/drivers/usbkbd/unbind

# Bind to your driver
echo -n "1-2:1.0" | sudo tee /sys/bus/usb/drivers/usbhid/bind

