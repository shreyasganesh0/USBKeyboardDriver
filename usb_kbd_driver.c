#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define CAPS_CODE 0x39

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/types.h>

#define DRIVER_VERSION ""
#define DRIVER_AUTHOR "Shreyas Ganesh"
#define DRIVER_DESC "USB HID Boot Protocol keyboard driver with dual mode"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

static const unsigned char usb_kbd_keycode[256] = {
	  0,  0,  0,  0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
	 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44,  2,  3,
	  4,  5,  6,  7,  8,  9, 10, 11, 28,  1, 14, 15, 57, 12, 13, 26,
	 27, 43, 43, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
	 65, 66, 67, 68, 87, 88, 99, 70,119,110,102,104,111,107,109,106,
	105,108,103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
	 72, 73, 82, 83, 86,127,116,117,183,184,185,186,187,188,189,190,
	191,192,193,194,134,138,130,132,128,129,131,137,133,135,136,113,
	115,114,  0,  0,  0,121,  0, 89, 93,124, 92, 94, 95,  0,  0,  0,
	122,123, 90, 91, 85,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 29, 42, 56,125, 97, 54,100,126,164,166,165,163,161,115,114,113,
	150,158,159,128,136,177,178,176,142,152,173,140
};


struct usb_kbd {
    uint8_t mode;
    uint8_t caps_count;
    uint8_t caps_pressed;
	spinlock_t caps_lock;

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

	spinlock_t leds_lock;
	bool led_urb_submitted;

};

static void usb_kbd_irq(struct urb *urb)
{
	struct usb_kbd *kbd = urb->context;
	unsigned long flags;
	int i;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		goto resubmit;
	}

	for (i = 0; i < 8; i++)
		input_report_key(kbd->dev, usb_kbd_keycode[i + 224], (kbd->new[0] >> i) & 1);

    for (i = 2; i < 8; i++) {

        //release check
		if (kbd->old[i] > 3 && memscan(kbd->new + 2, kbd->old[i], 6) == kbd->new + 8) {
			if (usb_kbd_keycode[kbd->old[i]]) {
                if (usb_kbd_keycode[kbd->old[i]] == usb_kbd_keycode[CAPS_CODE]) {

                    spin_lock_irqsave(&kbd->caps_lock, flags);
                    if (kbd->caps_pressed) {

                        kbd->caps_pressed = 0;
                        kbd->caps_count++;
                    }

                    if (kbd->caps_count == 4) {

                        input_report_key(kbd->dev, usb_kbd_keycode[kbd->old[i]], 0);
                        input_report_key(kbd->dev, usb_kbd_keycode[kbd->old[i]], 1);
                        kbd->mode = !kbd->mode; 
                        kbd->caps_count = 0;
                    }
                    spin_unlock_irqrestore(&kbd->caps_lock, flags);
                }

                input_report_key(kbd->dev, usb_kbd_keycode[kbd->old[i]], 0);
            } else {

				hid_info(urb->dev, "Unknown key (scancode %#x) released.\n", kbd->old[i]);
            }
		}

        //press check
		if (kbd->new[i] > 3 && memscan(kbd->old + 2, kbd->new[i], 6) == kbd->old + 8) {
			if (usb_kbd_keycode[kbd->new[i]]) {

                spin_lock_irqsave(&kbd->caps_lock, flags);

                if (usb_kbd_keycode[kbd->new[i]] == usb_kbd_keycode[CAPS_CODE]) {

                    if (kbd->caps_pressed) {

                        kbd->caps_count = 0; //reset if press exists for 2 events
                    }

                kbd->caps_pressed = 1;
                } else {

                  kbd->caps_count = 0; //reset if press exists for 2 events
                }
                
                spin_unlock_irqrestore(&kbd->caps_lock, flags);

                input_report_key(kbd->dev, usb_kbd_keycode[kbd->new[i]], 1);
            } else {

				hid_info(urb->dev, "Unknown key (scancode %#x) pressed.\n", kbd->new[i]);
            }
		}
	}

	input_sync(kbd->dev);

	memcpy(kbd->old, kbd->new, 8);

resubmit:
	i = usb_submit_urb (urb, GFP_ATOMIC);
	if (i)
		hid_err(urb->dev, "can't resubmit intr, %s-%s/input0, status %d",
			kbd->usbdev->bus->bus_name,
			kbd->usbdev->devpath, i);
}

static int usb_kbd_event(struct input_dev *dev, unsigned int type,
			 unsigned int code, int value)
{
	unsigned long flags;
	struct usb_kbd *kbd = input_get_drvdata(dev);

	if (type != EV_LED)
		return -1;

    spin_lock_irqsave(&kbd->caps_lock, flags);
    if (code == LED_CAPSL && kbd->mode == 1) {value = !value;} // invert value if mode = 1 (mode2)
	spin_unlock_irqrestore(&kbd->caps_lock, flags);

	spin_lock_irqsave(&kbd->leds_lock, flags);
    if (code == LED_CAPSL) {
	kbd->newleds = (!!test_bit(LED_KANA,    dev->led) << 4) | (!!test_bit(LED_COMPOSE, dev->led) << 3) |
		       (!!test_bit(LED_SCROLLL, dev->led) << 2) | 
		       ((value) << 1) | 
		       (!!test_bit(LED_NUML,    dev->led));
    } else {
        kbd->newleds = (!!test_bit(LED_KANA,    dev->led) << 4) | (!!test_bit(LED_COMPOSE, dev->led) << 3) |
                   (!!test_bit(LED_SCROLLL, dev->led) << 2) | 
                   (!!test_bit(LED_CAPSL, dev->led) << 1) | 
                   (!!test_bit(LED_NUML,    dev->led));
    }


	if (kbd->led_urb_submitted){
		spin_unlock_irqrestore(&kbd->leds_lock, flags);
		return 0;
	}

	if (*(kbd->leds) == kbd->newleds){
		spin_unlock_irqrestore(&kbd->leds_lock, flags);
		return 0;
	}

	*(kbd->leds) = kbd->newleds;

	kbd->led->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->led, GFP_ATOMIC))
		pr_err("usb_submit_urb(leds) failed\n");
	else
		kbd->led_urb_submitted = true;

	spin_unlock_irqrestore(&kbd->leds_lock, flags);

	return 0;
}

static void usb_kbd_led(struct urb *urb)
{
	unsigned long flags;
	struct usb_kbd *kbd = urb->context;

	if (urb->status)
		hid_warn(urb->dev, "led urb status %d received\n",
			 urb->status);

	spin_lock_irqsave(&kbd->leds_lock, flags);

	if (*(kbd->leds) == kbd->newleds){
		kbd->led_urb_submitted = false;
		spin_unlock_irqrestore(&kbd->leds_lock, flags);
		return;
	}

	*(kbd->leds) = kbd->newleds;

	kbd->led->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->led, GFP_ATOMIC)){
		hid_err(urb->dev, "usb_submit_urb(leds) failed\n");
		kbd->led_urb_submitted = false;
	}
	spin_unlock_irqrestore(&kbd->leds_lock, flags);

}

static int usb_kbd_open(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	kbd->irq->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->irq, GFP_KERNEL))
		return -EIO;

	return 0;
}

static void usb_kbd_close(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	usb_kill_urb(kbd->irq);
}

static int usb_kbd_alloc_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	if (!(kbd->irq = usb_alloc_urb(0, GFP_KERNEL)))
		return -1;
	if (!(kbd->led = usb_alloc_urb(0, GFP_KERNEL)))
		return -1;
	if (!(kbd->new = usb_alloc_coherent(dev, 8, GFP_KERNEL, &kbd->new_dma)))
		return -1;
	if (!(kbd->cr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL)))
		return -1;
	if (!(kbd->leds = usb_alloc_coherent(dev, 1, GFP_KERNEL, &kbd->leds_dma)))
		return -1;

	return 0;
}

static void usb_kbd_free_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	usb_free_urb(kbd->irq);
	usb_free_urb(kbd->led);
	usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
	kfree(kbd->cr);
	usb_free_coherent(dev, 1, kbd->leds, kbd->leds_dma);
}

static int usb_kbd_probe(struct usb_interface *iface,
			 const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(iface);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_kbd *kbd;
	struct input_dev *input_dev;
	int i, pipe, maxp;
	int error = -ENOMEM;

	interface = iface->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe);

	kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!kbd || !input_dev)
		goto fail1;

	if (usb_kbd_alloc_mem(dev, kbd))
		goto fail2;

	kbd->usbdev = dev;
	kbd->dev = input_dev;
	spin_lock_init(&kbd->leds_lock);

    kbd->caps_count = 0;
    kbd->caps_pressed = 0;
    kbd->mode = 0;
	spin_lock_init(&kbd->caps_lock);

	if (dev->manufacturer)
		strscpy(kbd->name, dev->manufacturer, sizeof(kbd->name));

	if (dev->product) {
		if (dev->manufacturer)
			strlcat(kbd->name, " ", sizeof(kbd->name));
		strlcat(kbd->name, dev->product, sizeof(kbd->name));
	}

	if (!strlen(kbd->name))
		snprintf(kbd->name, sizeof(kbd->name),
			 "USB HIDBP Keyboard %04x:%04x",
			 le16_to_cpu(dev->descriptor.idVendor),
			 le16_to_cpu(dev->descriptor.idProduct));

	usb_make_path(dev, kbd->phys, sizeof(kbd->phys));
	strlcat(kbd->phys, "/input0", sizeof(kbd->phys));

	input_dev->name = kbd->name;
	input_dev->phys = kbd->phys;
	usb_to_input_id(dev, &input_dev->id);
	input_dev->dev.parent = &iface->dev;

	input_set_drvdata(input_dev, kbd);

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) |
		BIT_MASK(EV_REP);
	input_dev->ledbit[0] = BIT_MASK(LED_NUML) | BIT_MASK(LED_CAPSL) |
		BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_COMPOSE) |
		BIT_MASK(LED_KANA);

	for (i = 0; i < 255; i++)
		set_bit(usb_kbd_keycode[i], input_dev->keybit);
	clear_bit(0, input_dev->keybit);

	input_dev->event = usb_kbd_event;
	input_dev->open = usb_kbd_open;
	input_dev->close = usb_kbd_close;

	usb_fill_int_urb(kbd->irq, dev, pipe,
			 kbd->new, (maxp > 8 ? 8 : maxp),
			 usb_kbd_irq, kbd, endpoint->bInterval);
	kbd->irq->transfer_dma = kbd->new_dma;
	kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	kbd->cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
	kbd->cr->bRequest = 0x09;
	kbd->cr->wValue = cpu_to_le16(0x200);
	kbd->cr->wIndex = cpu_to_le16(interface->desc.bInterfaceNumber);
	kbd->cr->wLength = cpu_to_le16(1);

	usb_fill_control_urb(kbd->led, dev, usb_sndctrlpipe(dev, 0),
			     (void *) kbd->cr, kbd->leds, 1,
			     usb_kbd_led, kbd);
	kbd->led->transfer_dma = kbd->leds_dma;
	kbd->led->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	error = input_register_device(kbd->dev);
	if (error)
		goto fail2;

	usb_set_intfdata(iface, kbd);
	device_set_wakeup_enable(&dev->dev, 1);
	return 0;

fail2:
	usb_kbd_free_mem(dev, kbd);
fail1:
	input_free_device(input_dev);
	kfree(kbd);
	return error;
}

static void usb_kbd_disconnect(struct usb_interface *intf)
{
	struct usb_kbd *kbd = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (kbd) {
		usb_kill_urb(kbd->irq);
		input_unregister_device(kbd->dev);
		usb_kill_urb(kbd->led);
		usb_kbd_free_mem(interface_to_usbdev(intf), kbd);
		kfree(kbd);
	}
}

static const struct usb_device_id usb_kbd_id_table[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ }						/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, usb_kbd_id_table);

static struct usb_driver usb_kbd_driver = {
	.name =		"usbkbd",
	.probe =	usb_kbd_probe,
	.disconnect =	usb_kbd_disconnect,
	.id_table =	usb_kbd_id_table,
};
static int __init usbkbd_init(void)
{
    return usb_register(&usb_kbd_driver);
}

static void __exit usbkbd_exit(void)
{
    usb_deregister(&usb_kbd_driver);
}

module_init(usbkbd_init);
module_exit(usbkbd_exit);
MODULE_LICENSE("GPL");
