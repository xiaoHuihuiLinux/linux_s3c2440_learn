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


static struct fasync_struct *button_async;
static struct class *seventh_class;
static struct class_device	*seventh_class_dev;
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;
/*��������ʱ��0x01 0x02 0x03 0x04*/
/*�����ɿ�ʱ��0x81 0x82 0x83 0x84*/
static unsigned char key_val;
static struct timer_list buttons_timer;
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/*�ж��¼���־���жϷ����������Ϊ1 seventh_drv_read������0*/
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
static struct pin_desc*irq_pd;
static DECLARE_MUTEX (button_lock); //���廥����

/*����������״̬*/
static irqreturn_t buttons_irq(int irq,void *dev_id)//�жϷ������Ĳ������жϺź�dev_id
{
	irq_pd  = (struct pin_desc*)dev_id;
	//10ms��������ʱ��
	//����jiffies 50 .HZ 100����ʱ��buttons_timer�ĳ�ʱʱ���� jiffies+HZ/100 =51  .ϵͳʱ����10ms����һ�����жϣ�Ȼ��jiffies��һ
	//jiffies ��Ϊ51 ϵͳʱ���жϴ�����ȥ�ҵ���Ӧ���Ѿ�����ʱ��Ķ�ʱ��
	mod_timer(&buttons_timer,jiffies+HZ/100);//jiffies ��ȫ�ֱ��� 
	return IRQ_RETVAL(IRQ_HANDLED);//��һ�β�֪��ΪʲĪż���Ĵ�ӡ���������Ƿ���return IRQ_HANDLED; ����
}
static unsigned int seventh_drv_poll(struct file *file, struct poll_table *wait)
{
	unsigned int mask =0;
	//p->qproc(filp, wait_address, p);
	poll_wait(file,&button_waitq,wait);//������������      �õ�ǰ���̹���button_waitq����
	if(ev_press)//��Ӧ�ó��򴥷�
		mask |= POLLIN | POLLRDNORM;
	return mask;//count����Ӽ� �����뿴����ѧϰ�ĵ�
		
}
static int seventh_drv_open(struct inode *inode,struct file *file)//file�������Ľṹ���Աf_flags��ʾ�Ƿ�������
{
	if(file->f_flags & O_NONBLOCK)//������ Ҳ����������
	{
		if(down_trylock(&button_lock))
			return -EBUSY;
	}
	else
	{
		/*��ȡ�ź���*/
		down(&button_lock);//����ź�����ȡ���� ���ͻ�����
	}

	/*ʹ���жϷ�ʽ*/
	//set_irq_chip(irqno, &s3c_irqext_chip); �ο�s3c_irqext_chip
	//int request_irq(unsigned int irq,irq_handler_t handler,unsigned long irqflags, const char * devname, void *dev_id)
	request_irq(IRQ_EINT0,buttons_irq,IRQT_BOTHEDGE,"s2",&pins_desc[0]);
	request_irq(IRQ_EINT2,buttons_irq,IRQT_BOTHEDGE,"s3",&pins_desc[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE,"s4",&pins_desc[2]);
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE,"s5",&pins_desc[3]);
	return 0;
}
ssize_t seventh_drv_read(struct file *file, char __user *buff,  size_t size, loff_t *ppos)
{
	if(size != 1)
		return -EINVAL;
	if(file->f_flags & O_NONBLOCK)//������ Ҳ����������
	{
		if(!ev_press)
			return -EAGAIN;
	}
	else
	{
		wait_event_interruptible(button_waitq,ev_press);//ev_press Ϊ0 ���̾ͻ�����
	}
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
	//wait_event_interruptible(button_waitq,ev_press);//ev_press Ϊ0 ���̾ͻ�����
	/*�а����Ļ����ؼ�ֵ*/
	copy_to_user(buff,&key_val,1);//�ڶ�������ʱָ��
	ev_press = 0;//������Ҫ�����־
	return 1;
}
static ssize_t seventh_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	return 0;
}
/*int (*release) (struct inode *, struct file *);*/
int seventh_drv_close (struct inode *inode, struct file *file)
{
	//up(&button_lock);
	free_irq(IRQ_EINT0,&pins_desc[0]);//ȡÿ���ĵ�ַ������ȡ����ĵ�ַ
	free_irq(IRQ_EINT2,&pins_desc[1]);
	free_irq(IRQ_EINT11,&pins_desc[2]);
	free_irq(IRQ_EINT19,&pins_desc[3]);
	up(&button_lock);
	return 0;
}
static int seventh_drv_fasync(int fd,struct file*filp,int on)//���Բο��ں˵�rtc_fasync
{
	printk("driver:seventh_drv_fasync\n");
	return fasync_helper(fd,filp,on,&button_async); //��ʼ��	button_async
}
static struct file_operations seventh_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =  seventh_drv_open,     
    .read	=   seventh_drv_read,	 //  .read	=   seventh_drv_read �����read	 ����д��write ���ʵ������ӡ
	.release = seventh_drv_close,
	.poll = seventh_drv_poll,
	.fasync = seventh_drv_fasync,
};
static void buttons_timer_function(unsigned long data)
{
	struct pin_desc*pindesc = irq_pd;
	/*��ǰ��һ���ֶ������жϴ���������ɵ�*/
	unsigned int pinval;
	if(!pindesc)//��Ϊ������seventh_drv_init����expires��ʱʱ����0���Բ��ܰ����Ƿ��¶���ִ�ж�ʱ��������������Ҫ�ж��Ƿ��а����¼�
		return;
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
	//kill_fasync(struct fasync_struct **fp, int sig, int band)
	//button_async �ĳ�ʼ�� ��seventh_drv_fasync���
	kill_fasync(&button_async,SIGIO,POLL_IN);//�жϷ�����Ӧ�ó���SIGIO�ź� 
	//////////////////////
}
int major;
static int seventh_drv_init(void)
{
	//��ʼ����ʱ��
	init_timer(&buttons_timer);
	//buttons_timer.data = (unsigned int )SCpnt;
	//buttons_timer.expires = jiffies + 100*HZ;/*10s*/ //��ʱ����ʱʱ������û�������൱��0
	
	buttons_timer.function = buttons_timer_function;//data�Ǹ�function���ݵĲ���
	add_timer(&buttons_timer);//�����ں�jiffies >0
	
		
	major = register_chrdev(0,"seventh_drv",&seventh_drv_fops);//�������豸��
	
	seventh_class = class_create(THIS_MODULE, "seventh_drv");
	/*�����ں˻��Զ�mdev���ں˵��ø���sysfs�µ���Ϣ����buttons�豸�ڵ� */
	seventh_class_dev = class_device_create(seventh_class, NULL, MKDEV(major, 0), NULL, "buttons"); 
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;//��Ϊ0x56000054
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;//��Ϊ0x56000054
	return 0; 
}
static void seventh_drv_exit(void)
{
	unregister_chrdev(major,"seventh_drv");
	class_device_unregister(seventh_class_dev);
	class_destroy(seventh_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}
module_init(seventh_drv_init);//������ں���
module_exit(seventh_drv_exit);//
MODULE_LICENSE("GPL");

