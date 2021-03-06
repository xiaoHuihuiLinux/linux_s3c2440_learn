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

static struct class *third_class;
static struct class_device	*third_class_dev;
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

static irqreturn_t buttons_irq(int irq,void *dev_id)
{
	printk("irq = %d\n",irq);//不是printf
	return IRQ_HANDLED; 
}

static int third_drv_open(struct inode *inode,struct file *file)
{
	#if 0
	//轮询的方式
	/*配置GPf0 2为输入引脚*/
	*gpfcon  &= ~((0x03 << (0 * 2 )) |(0x03 << (2 * 2 )));
	
	/*配置GPg3 11 为输入引脚*/
	*gpgcon  &= ~((0x03 << (3 * 2 )) |(0x03 << (11 * 2 )));
	#endif
	/*使用中断方式*/
	//set_irq_chip(irqno, &s3c_irqext_chip); 参考s3c_irqext_chip
	//int request_irq(unsigned int irq,irq_handler_t handler,unsigned long irqflags, const char * devname, void *dev_id)
	request_irq(IRQ_EINT0,buttons_irq,IRQT_BOTHEDGE,"s2",1);
	request_irq(IRQ_EINT2,buttons_irq,IRQT_BOTHEDGE,"s3",1);
	request_irq(IRQ_EINT11,buttons_irq,IRQT_BOTHEDGE,"s4",1);
	request_irq(IRQ_EINT19,buttons_irq,IRQT_BOTHEDGE,"s5",1);
	return 0;
}
ssize_t third_drv_read(struct file *filp, char __user *buff,  size_t size, loff_t *ppos)
{
	/*返回四个引脚的电平*/
	unsigned char key_vals[4];
	int regaval;
	if(size != sizeof(key_vals))
		return -EINVAL;
	/*读GPF 0 2*/
	regaval = *gpfdat;
	key_vals[0] = (regaval & (1 << 0)) ? 1 :0;
	key_vals[1] = (regaval & (1 << 2)) ? 1 :0;
	/*读GPF 3 11*/
	regaval = *gpgdat;
	key_vals[2] = (regaval & (1 << 3)) ? 1 :0;
	key_vals[3] = (regaval & (1 << 11)) ? 1 :0;
	copy_to_user(buff,key_vals,sizeof(key_vals));//返回给用户空间
	return sizeof(key_vals);
}
static ssize_t third_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	return 0;
}
/*int (*release) (struct inode *, struct file *);*/
int third_drv_close (struct inode *inode, struct file *file)
{
	free_irq(IRQ_EINT0,1);
	free_irq(IRQ_EINT2,1);
	free_irq(IRQ_EINT11,1);
	free_irq(IRQ_EINT19,1);
	return 0;
}

static struct file_operations third_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =  third_drv_open,     
    .read	=   third_drv_read,	 //  .read	=   third_drv_read 如果将read	 错误写成write 这个实验会疯狂打印
	.release = third_drv_close,
};
int major;
static int third_drv_init(void)
{
	major = register_chrdev(0,"third_drv",&third_drv_fops);//返回主设备号
	
	third_class = class_create(THIS_MODULE, "third_drv");
	/*这样内核会自动mdev被内核调用根据sysfs下的信息创建buttons设备节点 */
	third_class_dev = class_device_create(third_class, NULL, MKDEV(major, 0), NULL, "buttons"); 
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;//因为0x56000054
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;//因为0x56000054
	return 0; 
}
static void third_drv_exit(void)
{
	unregister_chrdev(major,"third_drv");
	class_device_unregister(third_class_dev);
	class_destroy(third_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}
module_init(third_drv_init);//修饰入口函数
module_exit(third_drv_exit);//
MODULE_LICENSE("GPL");

