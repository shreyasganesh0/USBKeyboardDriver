#!/bin/bash

./unbind.sh

sudo rmmod usb_kbd_driver

make clean

make

sudo insmod usb_kbd_driver.ko

./bind.sh
