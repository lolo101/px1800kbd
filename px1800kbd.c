/*
 * This software is forked from Colin Svingen's Azio L70 USB Keyboard driver
 * that can be found at https://bitbucket.org/Swoogan/aziokbd
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so by email.
 * Mail your message to Loïc Broquet <lbroquet@online.fr>
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt "\n"

#include <linux/module.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

/*
 * Driver Information
 */
#define DRIVER_AUTHOR "Loïc Broquet <lbroquet@online.fr>"
#define DRIVER_DESC "Perixx PX-1800 Keyboard Driver"
#define DRIVER_LICENSE "GPL"
#define ML_VENDOR_ID   0x0c45
#define ML_PRODUCT_ID  0x7603

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

#define debug(idev, msg, keys)		dev_dbg(&idev->dev, msg " mode=%d %02X-%02X-%02X-%02X-%02X-%02X-%02X", keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], keys[6], keys[7]);

static const unsigned char px_kbd_keycode[224] = {
		/* BEGIN MODE 01 */
/* 0-7 */	KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_MEDIA, KEY_MUTE, KEY_PAUSE, KEY_PREVIOUSSONG, KEY_PLAYPAUSE, KEY_NEXTSONG,
/* 8-15 */	KEY_MAIL, KEY_HOMEPAGE, KEY_CALC, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 16-23 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 24-31 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 32-39 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 40-47 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 48-55 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
		/* END MODE 01 */

		/* BEGIN MODE 04 */
/* 56-63 */	KEY_LEFTCTRL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LEFTMETA, KEY_RIGHTCTRL, KEY_RIGHTSHIFT, KEY_RIGHTALT, KEY_RESERVED,
/* 64-71 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_A, KEY_B, KEY_C, KEY_D,
/* 72-79 */	KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L,
/* 80-87 */	KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
/* 88-95 */	KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2,
/* 96-103 */	KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
/* 104-111 */	KEY_ENTER, KEY_ESC, KEY_BACKSPACE, KEY_TAB, KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_LEFTBRACE,
		/* END MODE 04 */

		/* BEGIN MODE 05 */
/* 112-119 */	KEY_RIGHTBRACE, KEY_BACKSLASH, KEY_BACKSLASH, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE, KEY_COMMA, KEY_DOT,
/* 120-127 */	KEY_SLASH, KEY_CAPSLOCK, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
/* 128-135 */	KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_SYSRQ, KEY_SCROLLLOCK,
/* 136-143 */	KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_PAGEUP, KEY_DELETE, KEY_END, KEY_PAGEDOWN, KEY_RIGHT,
/* 144-151 */	KEY_LEFT, KEY_DOWN, KEY_UP, KEY_NUMLOCK, KEY_KPSLASH, KEY_KPASTERISK, KEY_KPMINUS, KEY_KPPLUS,
/* 152-159 */	KEY_KPENTER, KEY_KP1, KEY_KP2, KEY_KP3, KEY_KP4, KEY_KP5, KEY_KP6, KEY_KP7,
/* 160-167 */	KEY_KP8, KEY_KP9, KEY_KP0, KEY_KPDOT, KEY_102ND, KEY_COMPOSE, KEY_RESERVED, KEY_RESERVED,
		/* END MODE 05 */

		/* BEGIN MODE 06 */
/* 168-175 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 176-183 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 184-191 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 192-199 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_SLASH,
/* 200-207 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 208-215 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
/* 216-223 */	KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED, KEY_RESERVED,
		/* END MODE 06 */
};

/**
 * struct usb_kbd - state of each attached keyboard
 * @dev:        input device associated with this keyboard
 * @usbdev:     usb device associated with this keyboard
 * @old:        data received in the past from the @irq URB representing which
 *              keys were pressed. By comparing with the current list of keys
 *              that are pressed, we are able to see key releases.
 * @irq:        URB for receiving a list of keys that are pressed when a
 *              new key is pressed or a key that was pressed is released.
 * @led:        URB for sending LEDs (e.g. numlock, ...)
 * @newleds:    data that will be sent with the @led URB representing which LEDs
                should be on
 * @name:       Name of the keyboard. @dev's name field points to this buffer
 * @phys:       Physical path of the keyboard. @dev's phys field points to this
 *              buffer
 * @new:        Buffer for the @irq URB
 * @cr:         Control request for @led URB
 * @leds:       Buffer for the @led URB
 * @new_dma:    DMA address for @irq URB
 * @leds_dma:   DMA address for @led URB
 * @leds_lock:  spinlock that protects @leds, @newleds, and @led_urb_submitted
 * @led_urb_submitted: indicates whether @led is in progress, i.e. it has been
 *              submitted and its completion handler has not returned yet
 *              without resubmitting @led
 */
struct usb_kbd {
	struct input_dev *dev;
	struct usb_device *usbdev;
	unsigned char old[8];
	struct urb *irq, *led;
	unsigned char newleds;
	char name[128];
	char phys[64];

	unsigned char *new;
	struct usb_ctrlrequest *cr;
	unsigned char *leds;
	dma_addr_t new_dma;
	dma_addr_t leds_dma;
};

static void check_key_pressed_released(struct usb_kbd *kbd, int value, int keycode)
{
	if (kbd->new[1] == value && kbd->old[1] != value)
		// key pressed
		input_report_key(kbd->dev, keycode, 1);
	if (kbd->old[1] == value && kbd->new[1] != value)
		// key released
		input_report_key(kbd->dev, keycode, 0);
}

static void scan_keys_pressed_released(struct usb_kbd *kbd, int mode)
{
	int i, j, offset;
	for (j = 1; j < 8; j++) {
		offset = (mode - 3) * 56 + (j - 1) * 8;
		for (i = 0; i < 8; i++)
			input_report_key(kbd->dev, px_kbd_keycode[offset + i], (kbd->new[j] >> i) & 1);
	}
}

static void handle_mode(struct usb_kbd *kbd)
{
	unsigned char mode = kbd->new[0];

	// The following lines are for logging keypresses to the
	// kernel dmesg facility. Build with DEBUG define
	// to capture the keycode for any non-functioning keys
	// and open a new issue on the project page with the key
	// you pressed and the keycode output below.
	debug(kbd->dev, "Keyup   keycode:", kbd->old);
	debug(kbd->dev, "Keydown keycode:", kbd->new);

	if (mode == 1) {
		check_key_pressed_released(kbd, 35, KEY_HOMEPAGE);
		check_key_pressed_released(kbd, 131, KEY_MEDIA);
		check_key_pressed_released(kbd, 138, KEY_MAIL);
		check_key_pressed_released(kbd, 146, KEY_CALC);
		check_key_pressed_released(kbd, 181, KEY_NEXTSONG);
		check_key_pressed_released(kbd, 182, KEY_PREVIOUSSONG);
		check_key_pressed_released(kbd, 183, KEY_PAUSE);
		check_key_pressed_released(kbd, 205, KEY_PLAYPAUSE);
		check_key_pressed_released(kbd, 226, KEY_MUTE);
		check_key_pressed_released(kbd, 233, KEY_VOLUMEUP);
		check_key_pressed_released(kbd, 234, KEY_VOLUMEDOWN);
	}
	else {
		scan_keys_pressed_released(kbd, mode);
	}

	input_sync(kbd->dev);

	memcpy(kbd->old, kbd->new, 8);
}

static void resubmit(struct urb *urb)
{
	int status = usb_submit_urb (urb, GFP_ATOMIC);
	if (status) {
		struct usb_kbd *kbd = urb->context;
		struct usb_device * usbdev = kbd->usbdev;
		hid_err(urb->dev, "can't resubmit intr, %s-%s/input0, status %d",
			usbdev->bus->bus_name,
			usbdev->devpath, status);
	}
}

static void usb_kbd_irq(struct urb *urb)
{
	switch (urb->status) {
	case 0:			/* success */
		handle_mode(urb->context);
		resubmit(urb);
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		break;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		resubmit(urb);
		break;
	}
}

static int usb_kbd_open(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	kbd->irq->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->irq, GFP_KERNEL))
		return -EIO;

	return 0;
}

static int update_leds(struct usb_kbd *kbd)
{
	if (*(kbd->leds) == kbd->newleds)
		return 0;

	*(kbd->leds) = kbd->newleds;
	kbd->led->dev = kbd->usbdev;
	return usb_submit_urb(kbd->led, GFP_ATOMIC);
}

static int kbd_led_event(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	kbd->newleds =
		(!!test_bit(LED_KANA,    dev->led) << 3) | (!!test_bit(LED_COMPOSE, dev->led) << 3) |
		(!!test_bit(LED_SCROLLL, dev->led) << 2) | (!!test_bit(LED_CAPSL,   dev->led) << 1) |
		(!!test_bit(LED_NUML,    dev->led));

	if (kbd->led->status == -EINPROGRESS)
		return 0;

	if (update_leds(kbd))
		hid_err(dev, "usb_submit_urb(leds) failed\n");

	return 0;
}

static int usb_kbd_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
	return type == EV_LED ? kbd_led_event(dev) : -EPERM;
}

static void usb_kbd_close(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	usb_kill_urb(kbd->irq);
}

static void usb_kbd_led(struct urb *urb)
{
	if (urb->status)
		hid_warn(urb->dev, "led urb status %d received\n", urb->status);

	if (update_leds(urb->context))
		hid_err(urb->dev, "usb_submit_urb(leds) failed\n");
}

static struct usb_kbd __must_check *usb_kbd_alloc_mem(struct usb_device *dev)
{
	struct usb_kbd *kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
	if (kbd &&
			(kbd->irq = usb_alloc_urb(0, GFP_KERNEL)) &&
			(kbd->led = usb_alloc_urb(0, GFP_KERNEL)) &&
			(kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma)) &&
			(kbd->cr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL)) &&
			(kbd->leds = usb_alloc_coherent(dev, 1, GFP_ATOMIC, &kbd->leds_dma)))
		return kbd;

	return NULL;
}

static void usb_kbd_free_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	usb_free_urb(kbd->irq);
	usb_free_urb(kbd->led);
	usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
	kfree(kbd->cr);
	usb_free_coherent(dev, 1, kbd->leds, kbd->leds_dma);
	kfree(kbd);
}

static void init_urb_transfer(struct urb* urb, dma_addr_t dmaAddr)
{
	urb->transfer_dma = dmaAddr;
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
}

static void init_kbd_name(struct usb_kbd *kbd)
{
	struct usb_device *dev = kbd->usbdev;

	if (dev->manufacturer)
		strlcpy(kbd->name, dev->manufacturer, sizeof(kbd->name));

	if (dev->product) {
		if (dev->manufacturer)
			strlcat(kbd->name, " ", sizeof(kbd->name));
		strlcat(kbd->name, dev->product, sizeof(kbd->name));
	}

	if (!strlen(kbd->name)) {
		struct usb_device_descriptor descriptor = dev->descriptor;
		snprintf(kbd->name, sizeof(kbd->name),
			 "USB HIDBP Keyboard %04x:%04x",
			 le16_to_cpu(descriptor.idVendor),
			 le16_to_cpu(descriptor.idProduct));
	}
}

static void init_kbd_phys(struct usb_kbd *kbd)
{
	struct usb_device *dev = kbd->usbdev;
	usb_make_path(dev, kbd->phys, sizeof(kbd->phys));
	strlcat(kbd->phys, "/input0", sizeof(kbd->phys));
}

static void init_kbd_irq(struct usb_kbd *kbd, struct usb_endpoint_descriptor *endpoint)
{
	struct usb_device *dev = kbd->usbdev;
	int pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	int maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
	struct urb* irq = kbd->irq;

	usb_fill_int_urb(irq, dev, pipe,
			 kbd->new, min(maxp, 8),
			 usb_kbd_irq, kbd, endpoint->bInterval);
	init_urb_transfer(irq, kbd->new_dma);
}

static void init_kbd_cr(struct usb_ctrlrequest *cr)
{
	cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
	cr->bRequest = 0x09;
	cr->wValue = cpu_to_le16(0x200);
	cr->wIndex = cpu_to_le16(0);
	cr->wLength = cpu_to_le16(1);
}

static void init_kbd_led(struct usb_kbd *kbd)
{
	struct usb_device *dev = kbd->usbdev;
	struct urb* led = kbd->led;
	usb_fill_control_urb(
		led, dev, usb_sndctrlpipe(dev, 0),
		(void *) kbd->cr, kbd->leds, 1,
		usb_kbd_led, kbd);
	init_urb_transfer(led, kbd->leds_dma);
}

static void init_kbd_dev(struct usb_kbd *kbd)
{
	int i;
	struct input_dev *input_dev = kbd->dev;

	input_dev->name = kbd->name;
	input_dev->phys = kbd->phys;

	input_set_drvdata(input_dev, kbd);

	input_dev->open  = usb_kbd_open;
	input_dev->event = usb_kbd_event;
	input_dev->close = usb_kbd_close;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) | BIT_MASK(EV_REP);
	input_dev->ledbit[0] = BIT_MASK(LED_NUML) | BIT_MASK(LED_CAPSL) | BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_COMPOSE) | BIT_MASK(LED_KANA);

	for (i = 0; i < sizeof(px_kbd_keycode); i++)
		set_bit(px_kbd_keycode[i], input_dev->keybit);

	clear_bit(KEY_RESERVED, input_dev->keybit);
}

static int usb_kbd_probe_endpoint(struct usb_interface *iface, struct usb_endpoint_descriptor *endpoint)
{
	int error = -ENOMEM;
	struct usb_device *dev = interface_to_usbdev(iface);
	struct usb_kbd *kbd = usb_kbd_alloc_mem(dev);

	if (kbd) {
		struct input_dev *input_dev = input_allocate_device();
		kbd->usbdev = dev;

		if (input_dev) {
			kbd->dev = input_dev;

			usb_to_input_id(dev, &input_dev->id);
			input_dev->dev.parent = &iface->dev;

			init_kbd_name(kbd);
			init_kbd_phys(kbd);
			init_kbd_irq(kbd, endpoint);
			init_kbd_cr(kbd->cr);
			init_kbd_led(kbd);
			init_kbd_dev(kbd);

			dev_info(&input_dev->dev, "detected %s", kbd->name);

			error = input_register_device(input_dev);
			if (error == 0) {
				usb_set_intfdata(iface, kbd);
				device_set_wakeup_enable(&dev->dev, 1);
				return 0;
			}
			input_free_device(input_dev);
		}
		usb_kbd_free_mem(dev, kbd);
	}
	return error;
}

static int usb_kbd_probe(struct usb_interface *iface, const struct usb_device_id *id)
{
	struct usb_host_interface *interface = iface->cur_altsetting;
	struct usb_endpoint_descriptor *endpoint = &interface->endpoint[0].desc;

	if (interface->desc.bNumEndpoints == 1 && usb_endpoint_is_int_in(endpoint))
		return usb_kbd_probe_endpoint(iface, endpoint);

	return -ENODEV;
}

static void usb_kbd_disconnect(struct usb_interface *intf)
{
	struct usb_kbd *kbd = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (kbd) {
		usb_kill_urb(kbd->irq);
		input_unregister_device(kbd->dev);
		usb_kbd_free_mem(interface_to_usbdev(intf), kbd);
	}
}

static struct usb_device_id usb_kbd_id_table [] = {
	{ USB_DEVICE(ML_VENDOR_ID, ML_PRODUCT_ID) },
	{ }						/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, usb_kbd_id_table);

static struct usb_driver usb_kbd_driver = {
	.name =		KBUILD_MODNAME,
	.probe =	usb_kbd_probe,
	.disconnect =	usb_kbd_disconnect,
	.id_table =	usb_kbd_id_table,
};

static int __init usb_kbd_init(void)
{
	int result = usb_register(&usb_kbd_driver);
	pr_info(DRIVER_DESC " registration status: %d", result);
	return result;
}

static void __exit usb_kbd_exit(void)
{
	usb_deregister(&usb_kbd_driver);
}

module_init(usb_kbd_init);
module_exit(usb_kbd_exit);
