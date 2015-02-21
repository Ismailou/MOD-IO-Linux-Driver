ARCH=arm
#CROSS_CMPILE=arm-unknown-linux-gnueabi-
CROSS_CMPILE=arm-linux-gnueabihf-

ifneq ($(KERNELRELEASE),)

	obj-m := mod_io_driver.o
#	objs-y := mod_io_driver.o twi.o
else
#	KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
	KERNEL_DIR ?= /usr/src/linux-header/
	PWD := $(shell pwd)
default:
#	$(MAKE) ARCH=$(ARCH) CROSS_CMPILE=$(CROSS_CMPILE) -C ${KERNEL_DIR} M=$(PWD) modules
	$(MAKE) ARCH=arm CC=arm-linux-gnueabihf-gcc -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules
	
clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order *.ko.unsigned *~
endif
