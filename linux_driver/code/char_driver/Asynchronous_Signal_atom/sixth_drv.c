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
/*按键按下时：0x01 0x02 0x03 0x04*/
/*按键松开时：0x81 0x82 0x83 0x84*/
static unsigned char key_val;
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/*中断事件标志，中断服务程序将他置为1 sixth_drv_read将他清0*/
static volatile int ev_press = 0;
struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};
struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0,0x01},//注意S3C2410_GPF0 SC等的大小写
	{S3C2410_GPF2,0x02},
	{S3C2410_GPG3,0x03},
	{S3C2410_GPG11,0x04},
};
	
static atomic_t canopen = ATOMIC_INIT(1);     //定义原子变量v并初始化为1
//static int canopen =1;
/*读出按键的状态*/
static irqreturn_t buttons_irq(int irq,void *dev_id)//中断服务程序的参数是中断号和dev_id
{
	struct pin_desc*pindesc = (struct pin_desc*)dev_id;
	unsigned int pinval;
	/*读取引脚值*/
	pinval = s3c2410_gpio_getpin(pindesc->pin);//跟直接操作dat寄存器是一样的
	if(pinval )
	{
		/*松开*/	
		key_val = 0x80 | pindesc->key_val;
	}
	else 
	{
		/*按下*/	
		key_val = pindesc->key_val;
	}
	ev_press = 1;//表示中断发生了
	wake_up_interruptible(&button_waitq);
	//kill_fasync(struct fasync_struct **fp, int sig, int band)
	//button_async 的初始化 在fith_drv_fasync完成
	kill_fasync(&button_async,SIGIO,POLL_IN);//中断发生给应用程序发SIGIO信号 
	return IRQ_RETVAL(IRQ_HANDLED);//第一次不知道为什莫偶发的打印按键按下是否是return IRQ_HANDLED; 导致
}
static unsigned int sixth_drv_poll(struct file *file, struct poll_table *wait)
{
	unsigned int mask =0;
	//p->qproc(filp, wait_address, p);
	poll_wait(file,&button_waitq,wait);//不会立即休眠      让当前进程挂在button_waitq队列
	if(ev_press)//有应用程序触发
		mask |= POLLIN | POLLRDNORM;
	return mask;//count不会加加 具体请看我们学习文档
		
}
static int sixth_drv_open(struct inode *inode,struct file *file)
{
	//假如A open的情况下 第一次进来的时候等于0。第二次进来就返回busy
	//但是linux是多任务的系统，有可能A 应用 执行成功 B也执行成功所以我们使用原子操作
	//第一次进来--canopen 为0 不满足if判断条件。执行注册中断。第二次的时候--canopen 为-1满足则进入而且atomic_inc 从新变为0
	if(!atomic_dec_and_test(&canopen)) //自减操作后测试其是否为0，为0则返回true，否则返回false。)
	{
		//canopen++;
		atomic_inc(&canopen);    //原子变量增加1
		return -EBUSY;
	}
	//试着if(!atomic_dec_and_test(&canopen));加分号猜猜会发生什么？
	/*使用中断方式*/
	//set_irq_chip(irqno, &s3c_irqext_chip); 参考s3c_irqext_chip
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
	/*没有按键动作休眠让出cpu*/
	/*
	#define wait_event_interruptible(wq, condition)				\
	({									\
	int __ret = 0;							\
	if (!(condition))						\
		__wait_event_interruptible(wq, condition, __ret);	\
	__ret;								\
	})
	*/
	wait_event_interruptible(button_waitq,ev_press);//ev_press 为0 进程就会休眠
	/*有按键的话返回键值*/
	copy_to_user(buff,&key_val,1);//第二个参数时指针
	ev_press = 0;//在这里要清掉标志
	return 1;
}
static ssize_t sixth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	return 0;
}
/*int (*release) (struct inode *, struct file *);*/
int sixth_drv_close (struct inode *inode, struct file *file)
{
	atomic_inc(&canopen);//从新让其变为1
	free_irq(IRQ_EINT0,&pins_desc[0]);//取每个的地址而不是取整体的地址
	free_irq(IRQ_EINT2,&pins_desc[1]);
	free_irq(IRQ_EINT11,&pins_desc[2]);
	free_irq(IRQ_EINT19,&pins_desc[3]);
	return 0;
}
static int fith_drv_fasync(int fd,struct file*filp,int on)//可以参考内核的rtc_fasync
{
	printk("driver:sixth_drv_fasync\n");
	return fasync_helper(fd,filp,on,&button_async); //初始化	button_async
}
static struct file_operations sixth_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =  sixth_drv_open,     
    .read	=   sixth_drv_read,	 //  .read	=   sixth_drv_read 如果将read	 错误写成write 这个实验会疯狂打印
	.release = sixth_drv_close,
	.poll = sixth_drv_poll,
	.fasync = fith_drv_fasync,
};
int major;
static int sixth_drv_init(void)
{
	major = register_chrdev(0,"sixth_drv",&sixth_drv_fops);//返回主设备号
	
	sixth_class = class_create(THIS_MODULE, "sixth_drv");
	/*这样内核会自动mdev被内核调用根据sysfs下的信息创建buttons设备节点 */
	sixth_class_dev = class_device_create(sixth_class, NULL, MKDEV(major, 0), NULL, "buttons"); 
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;//因为0x56000054
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;//因为0x56000054
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
module_init(sixth_drv_init);//修饰入口函数
module_exit(sixth_drv_exit);//
MODULE_LICENSE("GPL");

