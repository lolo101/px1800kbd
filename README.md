# Linux Microdia Keyboard Chipset Driver

For Chipset `0x0c45`:`0x7603`
The kernel reports the chipset as `Microdia`

Written for the Perixx PX-1800 USB Keyboard, also works for other keyboards with the same chipset (e.g. Periboard 506II)

This software is forked from Colin Svingen's Azio L70 USB Keyboard driver that can be found at https://bitbucket.org/Swoogan/aziokbd

**NOTE: Makefile and instructions have been tested on Debian and Arch Linux.**

# Installation

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

# Troubleshooting

## After reboot, the keyboard is not working

Check if the usbhid driver is in the initramfs:

    lsinitcpio /boot/initramfs-linux.img | grep usbhid

If the command returns something like:

    usr/lib/modules/<kernel version>/kernel/usbhid.ko

This means the usbhid driver is loaded early, before the file system is mounted. Therefore the usbhid configuration in `/etc/modprobe.d/usbhid.conf` cannot be read.

The solution is to add that configuration to the initramfs:

**WARNING: Be sure to understand what you are doing here. Corrupting you initramfs can prevent you system from booting**

    mkinitcpio -p linux

If properly configured, mkinitcpio should trigger the modconf hook which would automatically add the usbhid configuration into the initramfs.
