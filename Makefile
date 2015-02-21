ARCH=arm
#CROSS_CMPILE=arm-unknown-linux-gnueabi-
CROSS_COMPILE=arm-linux-gnueabihf-

ifneq ($(KERNELRELEASE),)

	obj-m := mod_io_dr.o
	mod_io_dr-y := mod_io_driver.o twi.o
else
#	KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
#	KERNEL_DIR ?= /usr/src/linux-header/
	KERNEL_DIR = /home/ismail/LinuxDriverProject/linux-sunxi/output/lib/modules/3.4.103-00033-g9a1cd03/build
	PWD := $(shell pwd)
default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ${KERNEL_DIR} M=$(PWD) modules
#	$(MAKE) ARCH=arm CC=arm-linux-gnueabihf-gcc -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *.ko.unsigned *~
endif
