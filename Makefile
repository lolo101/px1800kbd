# Uncomment the following to enable debug.
#DEBUG = y

KVER := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/input/keyboard
MODULE_NAME := px1800kbd
MODULE_VER := 1.0.0

ifeq ($(DEBUG),y)
	DBGFLAGS = -O -g -DML_DEBUG
else
	DBGFLAGS = -O2
endif

ccflags-y += $(DBGFLAGS)


ifneq ($(KERNELRELEASE),)
	obj-m := $(MODULE_NAME).o
else
	KSRC := /lib/modules/$(KVER)/build
	PWD := $(shell pwd)
endif

define REMOVE_MODULE
	@if [ -n "`dkms status $(MODULE_NAME)/$(MODULE_VER)`" ]; then \
		dkms remove $(MODULE_NAME)/$(MODULE_VER) --all; \
	fi;
endef

default:
	$(MAKE) -C $(KSRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KSRC) M=$(PWD) clean

uninstall:
	rm -f $(MODDESTDIR)/$(MODULE_NAME).ko
	/sbin/depmod -a ${KVER}

install:
	install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

dkms:  clean
	rm -rf /usr/src/$(MODULE_NAME)-$(MODULE_VER)
	mkdir --parent /usr/src/$(MODULE_NAME)-$(MODULE_VER)
	cp --target-directory=/usr/src/$(MODULE_NAME)-$(MODULE_VER) Makefile dkms.conf px1800kbd.c
	$(REMOVE_MODULE)
	dkms add -m $(MODULE_NAME) -v $(MODULE_VER)
	dkms build -m $(MODULE_NAME) -v $(MODULE_VER)
	dkms install -m $(MODULE_NAME) -v $(MODULE_VER) --force

