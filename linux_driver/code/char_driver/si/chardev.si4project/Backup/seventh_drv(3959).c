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
static struct class *sixth_class;
static struct class_device	*sixth_class_dev;
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;
/*��������ʱ��0x01 0x02 0x03 0x04*/
/*�����ɿ�ʱ��0x81 0x82 0x83 0x84*/
static unsigned char key_val;
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/*�ж��¼���־���жϷ����������Ϊ1 sixth_drv_read������0*/
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
	
static atomic_t canopen = ATOMIC_INIT(1);     //����ԭ�ӱ���v����ʼ��Ϊ1
//static int canopen =1;
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
	//kill_fasync(struct fasync_struct **fp, int sig, int band)
	//button_async �ĳ�ʼ�� ��fith_drv_fasync���
	kill_fasync(&button_async,SIGIO,POLL_IN);//�жϷ�����Ӧ�ó���SIGIO�ź� 
	return IRQ_RETVAL(IRQ_HANDLED);//��һ�β�֪��ΪʲĪż���Ĵ�ӡ���������Ƿ���return IRQ_HANDLED; ����
}
static unsigned int sixth_drv_poll(struct file *file, struct poll_table *wait)
{
	unsigned int mask =0;
	//p->qproc(filp, wait_address, p);
	poll_wait(file,&button_waitq,wait);//������������      �õ�ǰ���̹���button_waitq����
	if(ev_press)//��Ӧ�ó��򴥷�
		mask |= POLLIN | POLLRDNORM;
	return mask;//count����Ӽ� �����뿴����ѧϰ�ĵ�
		
}
static int sixth_drv_open(struct inode *inode,struct file *file)
{
	//����A open������� ��һ�ν�����ʱ�����0���ڶ��ν����ͷ���busy
	//����linux�Ƕ������ϵͳ���п���A Ӧ�� ִ�гɹ� BҲִ�гɹ���������ʹ��ԭ�Ӳ���
	//��һ�ν���--canopen Ϊ0 ������if�ж�������ִ��ע���жϡ��ڶ��ε�ʱ��--canopen Ϊ-1������������atomic_inc ���±�Ϊ0
	if(!atomic_dec_and_test(&canopen)) //�Լ�������������Ƿ�Ϊ0��Ϊ0�򷵻�true�����򷵻�false��)
	{
		//canopen++;
		atomic_inc(&canopen);    //ԭ�ӱ�������1
		return -EBUSY;
	}
	//����if(!atomic_dec_and_test(&canopen));�ӷֺŲ²»ᷢ��ʲô��
	/*ʹ���жϷ�ʽ*/
	//set_irq_chip(irqno, &s3c_irqext_chip); �ο�s3c_irqext_chip
	//int request_irq(unsigned int irq,irq_handler_t handler,unsigned long irqflags, const char * devname, void *dev_id)
	request_irq(IRQ_EINT0,buttons_irq,IRQT_BOTHEDGE,"s2",&pins_desc[0]);
	request_irq(IRQ_EINT2,buttons_irq,IRQT_BOTHEDGE,"s3",&pins_desc[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE,"s4",&pins_desc[2]);
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE,"s5",&pins_desc[3]);
	return 0;
}
ssize_t sixth_drv_read(struct file *filp, char __user *buff,  size_t size, loff_t *ppos)
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
static ssize_t sixth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	return 0;
}
/*int (*release) (struct inode *, struct file *);*/
int sixth_drv_close (struct inode *inode, struct file *file)
{
	atomic_inc(&canopen);//���������Ϊ1
	free_irq(IRQ_EINT0,&pins_desc[0]);//ȡÿ���ĵ�ַ������ȡ����ĵ�ַ
	free_irq(IRQ_EINT2,&pins_desc[1]);
	free_irq(IRQ_EINT11,&pins_desc[2]);
	free_irq(IRQ_EINT19,&pins_desc[3]);
	return 0;
}
static int fith_drv_fasync(int fd,struct file*filp,int on)//���Բο��ں˵�rtc_fasync
{
	printk("driver:sixth_drv_fasync\n");
	return fasync_helper(fd,filp,on,&button_async); //��ʼ��	button_async
}
static struct file_operations sixth_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =  sixth_drv_open,     
    .read	=   sixth_drv_read,	 //  .read	=   sixth_drv_read �����read	 ����д��write ���ʵ������ӡ
	.release = sixth_drv_close,
	.poll = sixth_drv_poll,
	.fasync = fith_drv_fasync,
};
int major;
static int sixth_drv_init(void)
{
	major = register_chrdev(0,"sixth_drv",&sixth_drv_fops);//�������豸��
	
	sixth_class = class_create(THIS_MODULE, "sixth_drv");
	/*�����ں˻��Զ�mdev���ں˵��ø���sysfs�µ���Ϣ����buttons�豸�ڵ� */
	sixth_class_dev = class_device_create(sixth_class, NULL, MKDEV(major, 0), NULL, "buttons"); 
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;//��Ϊ0x56000054
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;//��Ϊ0x56000054
	return 0; 
}
static void sixth_drv_exit(void)
{
	unregister_chrdev(major,"sixth_drv");
	class_device_unregister(sixth_class_dev);
	class_destroy(sixth_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}
module_init(sixth_drv_init);//������ں���
module_exit(sixth_drv_exit);//
MODULE_LICENSE("GPL");
