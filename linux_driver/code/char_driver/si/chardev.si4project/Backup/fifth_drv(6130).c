#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/poll.h>

static struct class *fouth_class;
static struct class_device	*fouth_class_dev;
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;
/*��������ʱ��0x01 0x02 0x03 0x04*/
/*�����ɿ�ʱ��0x81 0x82 0x83 0x84*/
static unsigned char key_val;
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/*�ж��¼���־���жϷ����������Ϊ1 fouth_drv_read������0*/
static volatile int ev_press = 0;
struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};
struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0,0x01},//ע��S3C2410_GPF0 SC�ȵĴ�Сд
	{S3C2410_GPF2,0x02},
	{S3C2410_GPG3,0x03},
	{S3C2410_GPG11,0x04},
};
/*����������״̬*/
static irqreturn_t buttons_irq(int irq,void *dev_id)//�жϷ������Ĳ������жϺź�dev_id
{
	struct pin_desc*pindesc = (struct pin_desc*)dev_id;
	unsigned int pinval;
	/*��ȡ����ֵ*/
	pinval = s3c2410_gpio_getpin(pindesc->pin);//��ֱ�Ӳ���dat�Ĵ�����һ����
	if(pinval )
	{
		/*�ɿ�*/	
		key_val = 0x80 | pindesc->key_val;
	}
	else 
	{
		/*����*/	
		key_val = pindesc->key_val;
	}
	ev_press = 1;//��ʾ�жϷ�����
	wake_up_interruptible(&button_waitq);
	//printk("irq = %d\n",irq);//����printf
	
	//return IRQ_HANDLED; 
	return IRQ_RETVAL(IRQ_HANDLED);//��һ�β�֪��ΪʲĪż���Ĵ�ӡ���������Ƿ���return IRQ_HANDLED; ����
}
static unsigned int fouth_drv_poll(struct file *file, struct poll_table *wait)
{
	unsigned int mask =0;
	//p->qproc(filp, wait_address, p);
	poll_wait(file,&button_waitq,wait);//������������      �õ�ǰ���̹���button_waitq����
	if(ev_press)//��Ӧ�ó��򴥷�
		mask |= POLLIN | POLLRDNORM;
	return mask;//count����Ӽ� �����뿴����ѧϰ�ĵ�
		
}
static int fouth_drv_open(struct inode *inode,struct file *file)
{
	#if 0
	//��ѯ�ķ�ʽ
	/*����GPf0 2Ϊ��������*/
	*gpfcon  &= ~((0x03 << (0 * 2 )) |(0x03 << (2 * 2 )));
	
	/*����GPg3 11 Ϊ��������*/
	*gpgcon  &= ~((0x03 << (3 * 2 )) |(0x03 << (11 * 2 )));
	#endif
	/*ʹ���жϷ�ʽ*/
	//set_irq_chip(irqno, &s3c_irqext_chip); �ο�s3c_irqext_chip
	//int request_irq(unsigned int irq,irq_handler_t handler,unsigned long irqflags, const char * devname, void *dev_id)
	request_irq(IRQ_EINT0,buttons_irq,IRQT_BOTHEDGE,"s2",&pins_desc[0]);
	request_irq(IRQ_EINT2,buttons_irq,IRQT_BOTHEDGE,"s3",&pins_desc[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE,"s4",&pins_desc[2]);
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE,"s5",&pins_desc[3]);
	return 0;
}
ssize_t fouth_drv_read(struct file *filp, char __user *buff,  size_t size, loff_t *ppos)
{
	if(size != 1)
		return -EINVAL;
	/*û�а������������ó�cpu*/
	/*
	#define wait_event_interruptible(wq, condition)				\
	({									\
	int __ret = 0;							\
	if (!(condition))						\
		__wait_event_interruptible(wq, condition, __ret);	\
	__ret;								\
	})
	*/
	wait_event_interruptible(button_waitq,ev_press);//ev_press Ϊ0 ���̾ͻ�����
	/*�а����Ļ����ؼ�ֵ*/
	copy_to_user(buff,&key_val,1);//�ڶ�������ʱָ��
	ev_press = 0;//������Ҫ�����־
	return 1;
}
static ssize_t fouth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	return 0;
}
/*int (*release) (struct inode *, struct file *);*/
int fouth_drv_close (struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0,&pins_desc[0]);//ȡÿ���ĵ�ַ������ȡ����ĵ�ַ
	free_irq(IRQ_EINT2,&pins_desc[1]);
	free_irq(IRQ_EINT11,&pins_desc[2]);
	free_irq(IRQ_EINT19,&pins_desc[3]);
	return 0;
}

static struct file_operations fouth_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =  fouth_drv_open,     
    .read	=   fouth_drv_read,	 //  .read	=   fouth_drv_read �����read	 ����д��write ���ʵ������ӡ
	.release = fouth_drv_close,
	.poll = fouth_drv_poll,
};
int major;
static int fouth_drv_init(void)
{
	major = register_chrdev(0,"fouth_drv",&fouth_drv_fops);//�������豸��
	
	fouth_class = class_create(THIS_MODULE, "fouth_drv");
	/*�����ں˻��Զ�mdev���ں˵��ø���sysfs�µ���Ϣ����buttons�豸�ڵ� */
	fouth_class_dev = class_device_create(fouth_class, NULL, MKDEV(major, 0), NULL, "buttons"); 
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;//��Ϊ0x56000054
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;//��Ϊ0x56000054
	return 0; 
}
static void fouth_drv_exit(void)
{
	unregister_chrdev(major,"fouth_drv");
	class_device_unregister(fouth_class_dev);
	class_destroy(fouth_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}
module_init(fouth_drv_init);//������ں���
module_exit(fouth_drv_exit);//
MODULE_LICENSE("GPL");

