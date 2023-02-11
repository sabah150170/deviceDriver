
obj-m += blg413-system.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	$(MAKE) -c $(KDIR) SUBDIRS=$(PWD) module_install

clean:
	$(MAKE) -c $(KDIR) SUBDIRS=$(PWD) clear
