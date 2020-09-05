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
//����Ҫʵ�����ǵ�������Ұ����Լ������¼��ο��ں˵�Usbmouse.c
struct input_dev *uk_dev;//����һ��input_dev�ṹ��
static char * usb_buf;
static dma_addr_t usb_buf_phys;
static struct urb* uk_urb;
static int len;
static void usbmouse_as_key_irq(struct urb *urb)//usb��������������cpu�����ж�
{
	int i;
	static int cnt = 0;
	printk("data cnt %d: ", ++cnt);
	for (i = 0; i < len; i++)
	{
		printk("%02x ", usb_buf[i]);
	}
	printk("\n");

	/* �����ύurb */
	usb_submit_urb(uk_urb, GFP_KERNEL);
}
static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	//struct usb_device *dev = interface_to_usbdev(intf);
	//printk("FoundMousKey��\n");
	//printk("bcdusb = %x\n",dev->descriptor.bcdUSB);
	//printk("VID = 0x%x\n",dev->descriptor.idVendor);
	//printk("PID = 0x%x\n",dev->descriptor.idProduct);
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	int pipe;
	interface = intf->cur_altsetting;
	endpoint = &interface->endpoint[0].desc;
	/*1����input_dev�ṹ��
	 *2���ò������¼�
	 *3ע��
	 *Ӳ���Ĳ���
	 */
	 //����ṹ��
	uk_dev = input_allocate_device();//����ṹ��
	//���������¼�
	set_bit(EV_KEY,uk_dev->evbit);//�����¼�
	set_bit(EV_REP,uk_dev->evbit);//�ظ����¼�P����REL
	//������Щ�¼�
	set_bit(KEY_L,uk_dev->keybit);//L
	set_bit(KEY_S,uk_dev->keybit);//S
	set_bit(KEY_ENTER,uk_dev->keybit);//Enter
	input_register_device(uk_dev);//ע��
	
	//Ӳ������ز�����ǰ�����������Ǵ��жϻ��߼Ĵ�������״̬ȷ��ʲô�������ڵ��������Եײ��usb�������������ṩ���շ�����
	/*�������Ҫ�أ�
	*1.Դ
	*2.����
	*3.Ŀ��
	*/
	//Դ��ĳ��usb�豸�Ķ˵�
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);//usb_rcvintpipe������ж��豸�źͶ˵��ַ
	//���ȣ��˵����������г���
	len = endpoint->wMaxPacketSize;
	//Ŀ�ģ����������ڴ�
	usb_buf = usb_buffer_alloc(dev, len, GFP_ATOMIC, &usb_buf_phys);
	//ʹ�õ���Ҫ�أ�
	/*
	*����urb
	*����urb
	*ʹ��urb
	*/
	uk_urb = usb_alloc_urb(0, GFP_KERNEL);//����
	usb_fill_int_urb(uk_urb, dev, pipe, usb_buf,len,usbmouse_as_key_irq, NULL, endpoint->bInterval);
	uk_urb->transfer_dma = usb_buf_phys;
	uk_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	//ʹ��urb
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
	usb_buffer_free(dev, len, usb_buf, usb_buf_phys);//free�Ĳ�����usb_buffer_alloc�е���������Ĳ���
	input_unregister_device(uk_dev);//ע��
	input_free_device(uk_dev);
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

