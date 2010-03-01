#
# Makefile for LabJack U3 kernel module
#
KDIR := /lib/modules/$(shell uname -r)/build
INSTDIR := $(DESTDIR)$(MODDIR)/kernel/drivers/labjack

obj-m := labjacku3.o

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:

clean:
	rm -fr *.mod.c *.o *.ko .labjack* .tmp*
