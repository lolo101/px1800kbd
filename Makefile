# Uncomment the following to enable debug.
#DEBUG = y

KVER := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/input/keyboard
MODULE_NAME := px1800kbd
MODULE_VER := 2.0.0-SNAPSHOT
MODSRCDIR := /usr/src/$(MODULE_NAME)-$(MODULE_VER)

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
	rm -rf ${MODSRCDIR}
	mkdir --parent ${MODSRCDIR}
	cp --target-directory=${MODSRCDIR} Makefile px1800kbd.c
	sed -e s/%MODULE_NAME%/${MODULE_NAME}/ -e s/%MODULE_VER%/$(MODULE_VER)/ dkms.conf > ${MODSRCDIR}/dkms.conf
	$(REMOVE_MODULE)
	dkms add -m $(MODULE_NAME) -v $(MODULE_VER)
	dkms build -m $(MODULE_NAME) -v $(MODULE_VER)
	dkms install -m $(MODULE_NAME) -v $(MODULE_VER) --force

