#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
/*我们要做的就是分配设置一个usb_driver结构体*/

/*USB_DEVICE这个宏定义在include/linux下*/
/*只要我们的接口类信息里面的东西是USB_INTERFACE_CLASS_HID USB_INTERFACE_SUBCLASS_BOOT
USB_INTERFACE_PROTOCOL_MOUSE 这个usb设备id_table就能支持*/
static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	//{USB_DEVICE(0x12d1,0x1001)},//假如我们想支持我们已经知道的VID:0x12d1PID:0x1001
	{ }	/* Terminating entry */
};

static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk("FoundMousKey！\n");
	return 0;
}
static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	printk("disconnect usbmouse!\n");
}
//1，分配一个 usb_driver 结构体。

static struct usb_driver usb_mouse_as_key_driver = {
	.name		= "usbmouse_as_key_",
	.probe		= usbmouse_as_key_probe,
	.disconnect	= usbmouse_as_key_disconnect,
	.id_table	= usbmouse_as_key_id_table,//查看是否支持设备
};

static int usb_mouse_as_key_init(void)
{
	usb_register(&usb_mouse_as_key_driver);
	return 0;
}
static void usb_mouse_as_key_exit(void)
{
	//2注册
	usb_deregister(&usb_mouse_as_key_driver);
}
module_init(usb_mouse_as_key_init);
module_exit(usb_mouse_as_key_exit);
MODULE_LICENSE("GPL");

