# Linux Microdia Keyboard Chipset Driver #

For Chipset `0x0c45`:`0x7603`
The kernel reports the chipset as `SONiX USB Keyboard`

Written for the Perixx PX-1800 USB Keyboard.
This software is forked from Colin Svingen's Azio L70 USB Keyboard driver that can be found at https://bitbucket.org/Swoogan/aziokbd

> NOTE: Makefile and instructions are only tested on Ubuntu, however they are known to work on Debian.

# Installation ##

First, make sure to have all the required packages to download and build the module:

    sudo apt-get install git build-essential linux-headers-generic dkms

Then, download the sources:

    git clone https://github.com/lolo101/px1800kbd.git

`cd` into the sources directory:

    cd px1800kbd

At last, build the module. It is recommended to use DKMS to install the module:

    sudo ./install.sh dkms

But you may also want to run a manual install:

    sudo ./install.sh

# Blacklisting #

**NOTE: install.sh attempts to blacklist the driver for you. You shouldn't need to do anything manually. These instructions are to explain the process, in the event something goes wrong.**

You need to blacklist the device from the generic USB hid module in order for the px1800kbd module to control it.

## Kernel Module ##

If the USB hid driver is compiled as a kernel module you will need to create a quirks file and blacklist it there.

You can determine if the module is a kernel module by running the following:

    lsmod | grep usbhid

If `grep` finds something, it means that the module is a kernel module.

Create a file called `/etc/modprobe.d/usbhid.conf` and add the following to it:

    options usbhid quirks=0x0c45:0x7603:0x00000004

For more information about quirks, see [`include/linux/hid.h`](https://github.com/torvalds/linux/blob/master/include/linux/hid.h).

## Compiled into Kernel ##

If the generic USB hid driver is compiled into the kernel, then the driver is not loaded as a module and setting the option via `modprobe` will not work. In this case you must pass the option to the driver via the grub boot loader.

Create a new file in `/etc/default/grub.d/`. For example, you might call it `px1800kbd.conf`. (If your grub package doesn't have this directory, just modify the generic `/etc/default/grub` configuration file):

    GRUB_CMDLINE_LINUX_DEFAULT='usbhid.quirks=0x0c45:0x7603:0x00000004'

Then run `sudo update-grub` and reboot.
