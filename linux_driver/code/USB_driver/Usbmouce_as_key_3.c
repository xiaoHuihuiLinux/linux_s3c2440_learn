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
//我们要实现我们的鼠标左右按键以及滚轮事件参考内核的Usbmouse.c
struct input_dev *uk_dev;//分配一个input_dev结构体
static char * usb_buf;
static dma_addr_t usb_buf_phys;
static struct urb* uk_urb;
static int len;
static void usbmouse_as_key_irq(struct urb *urb)//usb主机控制器控制cpu产生中断
{
	int i;
	static int cnt = 0;
	printk("data cnt %d: ", ++cnt);
	for (i = 0; i < len; i++)
	{
		printk("%02x ", usb_buf[i]);
	}
	printk("\n");

	/* 重新提交urb */
	usb_submit_urb(uk_urb, GFP_KERNEL);
}
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	//struct usb_device *dev = interface_to_usbdev(intf);
	//printk("FoundMousKey！\n");
	//printk("bcdusb = %x\n",dev->descriptor.bcdUSB);
	//printk("VID = 0x%x\n",dev->descriptor.idVendor);
	//printk("PID = 0x%x\n",dev->descriptor.idProduct);
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	int pipe;
	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;
	/*1分配input_dev结构体
	 *2设置产生的事件
	 *3注册
	 *硬件的操作
	 */
	 //分配结构体
	uk_dev = input_allocate_device();//分配结构体
	//产生哪类事件
	set_bit(EV_KEY,uk_dev->evbit);//按键事件
	set_bit(EV_REP,uk_dev->evbit);//重复类事件P不是REL
	//产生哪些事件
	set_bit(KEY_L,uk_dev->keybit);//L
	set_bit(KEY_S,uk_dev->keybit);//S
	set_bit(KEY_ENTER,uk_dev->keybit);//Enter
	input_register_device(uk_dev);//注册
	
	//硬件的相关操作以前的我们数据是从中断或者寄存器引脚状态确定什么数据现在的数据来自底层的usb总线驱动程序提供的收发函数
	/*传输的三要素：
	*1.源
	*2.长度
	*3.目的
	*/
	//源：某个usb设备的端点
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);//usb_rcvintpipe这个宏判断设备号和端点地址
	//长度：端点描述符中有长度
	len = endpoint->wMaxPacketSize;
	//目的：开辟物理内存
	usb_buf = usb_buffer_alloc(dev, len, GFP_ATOMIC, &usb_buf_phys);
	//使用的三要素：
	/*
	*分配urb
	*设置urb
	*使用urb
	*/
	uk_urb = usb_alloc_urb(0, GFP_KERNEL);//分配
	usb_fill_int_urb(uk_urb, dev, pipe, usb_buf,len,usbmouse_as_key_irq, NULL, endpoint->bInterval);
	uk_urb->transfer_dma = usb_buf_phys;
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	//使用urb
	usb_submit_urb(uk_urb, GFP_KERNEL);
	return 0;
}


static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
	//printk("disconnect usbmouse!\n");
	struct usb_device *dev = interface_to_usbdev(intf);
	usb_kill_urb(uk_urb);//usb_submit_urb
	usb_free_urb(uk_urb);
	/*
	void usb_buffer_free(
	struct usb_device *dev,
	size_t size,
	void *addr,
	dma_addr_t dma
	)
	*/
	usb_buffer_free(dev, len, usb_buf, usb_buf_phys);//free的参数和usb_buffer_alloc有点区别第三四参数
	input_unregister_device(uk_dev);//注册
	input_free_device(uk_dev);
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

