obj-m := waitqueue.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules

modules modules_install help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@
