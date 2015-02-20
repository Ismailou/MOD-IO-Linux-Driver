ARCH=arm
#CROSS_COMPILE=arm-unknown-linux-gnueabi-
CROSS_COMPILE=arm-linux-gnueabihf-
#CROSS_COMPILE=arm-Allwinner_A20-linux-gnueabi-

ifneq ($(KERNELRELEASE),)

	obj-m := Hello.o
#	mod-objs := mod_io_driver.o twi.o
#	Hello_World_4-objs := Hello_World_4_Start.o Hello_World_4_Stop.o
else
#	KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
	KERNEL_DIR ?= /home/ismail/LinuxDriverProject/linux-sunxi/output/lib/modules/3.4.103-00033-g9a1cd03/build
	PWD := $(shell pwd)
default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ${KERNEL_DIR} M=$(PWD) modules
#	$(MAKE) -C ${KERNEL_DIR} M=$(PWD) modules

clean:
	make -C ${KERNEL_DIR} M=$(PWD) clea
endif
