#!/bin/sh

sudo cp labjacku3.ko /lib/modules/`uname -r`/kernel/drivers/usb/misc/labjacku3.ko
sudo depmod -a
sudo modprobe labjacku3
sudo mkdir /dev/usb
sudo mknod --mode=a=rw /dev/usb/labjacku3_0 c 180 208

