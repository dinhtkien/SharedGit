obj-m := tasklet.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all default: modules

modules help clean:
	$(MAKE) -C $(KERNELDIR) M=$(shell pwd) $@

