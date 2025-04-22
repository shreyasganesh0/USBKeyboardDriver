# USB Keyboard Driver

USB Keyboard driver with 2 modes

# Usage

- To build the driver and bind new driver to the keyboard
```
./remake.sh
```
    - Note to make sure that this works use
    ```
    ls /sys/bus/usb/drivers
    ```
        to see if 'usbkbd' is present
    ```
    sudo dmesg | grep -i usb
    ```
        to find the interface to bind the driver to something like '1-2|1.0'

    - Replace the interface in place of '1-2:1.0' values in the ./bind.sh and ./unbind.sh files

- Alternatively build the driver using
```
make
```
- Install the driver
```
sudo insmod usb_kbd_driver
```
- Unbind the interface from the default driver
```
echo -n "1-2:1.0" | sudo tee /sys/bus/usb/drivers/usbhid/unbind
```
- Bind the interface to the new driver 
```
echo -n "1-2:1.0" | sudo tee /sys/bus/usb/drivers/usbkbd/bind
```
