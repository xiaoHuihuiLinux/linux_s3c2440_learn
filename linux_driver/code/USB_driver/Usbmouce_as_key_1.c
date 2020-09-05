#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
/*����Ҫ���ľ��Ƿ�������һ��usb_driver�ṹ��*/

/*USB_DEVICE����궨����include/linux��*/
/*ֻҪ���ǵĽӿ�����Ϣ����Ķ�����USB_INTERFACE_CLASS_HID USB_INTERFACE_SUBCLASS_BOOT
USB_INTERFACE_PROTOCOL_MOUSE ���usb�豸id_table����֧��*/
static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	//{USB_DEVICE(0x12d1,0x1001)},//����������֧�������Ѿ�֪����VID:0x12d1PID:0x1001
	{ }	/* Terminating entry */
};

static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk("FoundMousKey��\n");
	return 0;
}
static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	printk("disconnect usbmouse!\n");
}
//1������һ�� usb_driver �ṹ�塣

static struct usb_driver usb_mouse_as_key_driver = {
	.name		= "usbmouse_as_key_",
	.probe		= usbmouse_as_key_probe,
	.disconnect	= usbmouse_as_key_disconnect,
	.id_table	= usbmouse_as_key_id_table,//�鿴�Ƿ�֧���豸
};

static int usb_mouse_as_key_init(void)
{
	usb_register(&usb_mouse_as_key_driver);
	return 0;
}
static void usb_mouse_as_key_exit(void)
{
	//2ע��
	usb_deregister(&usb_mouse_as_key_driver);
}
module_init(usb_mouse_as_key_init);
module_exit(usb_mouse_as_key_exit);
MODULE_LICENSE("GPL");

