
obj-m := AT24C256.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules

modules help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
