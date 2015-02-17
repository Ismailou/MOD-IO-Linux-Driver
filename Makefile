ARCH=armv7l
CROSS_CMPILE=arm-unknown-linux-gnueabi

ifneq ($(KERNELRELEASE),)

	obj-m := mod_io_driver.o twi.o
#	Hello_World_4-objs := Hello_World_4_Start.o Hello_World_4_Stop.o
else
	KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) ARCH=$(ARCH) CROSS_CMPILE=$(CROSS_CMPILE) -C ${KERNEL_DIR} M=$(PWD) modules
endif
