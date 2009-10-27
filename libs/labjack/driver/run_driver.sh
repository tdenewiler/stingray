#!/bin/sh

sudo modprobe labjacku3
sudo mkdir /dev/usb
sudo mknod --mode=a=rw /dev/usb/labjacku3_0 c 180 208

