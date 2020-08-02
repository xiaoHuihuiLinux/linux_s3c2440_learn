#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *second_class;
static struct class_device	*second_class_dev;
volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;

volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;
static int second_drv_open(struct inode *inode,struct file *file)
{
	/*配置GPf0 2为输入引脚*/
	*gpfcon  &= ~((0x03 << (0 * 2 )) |(0x03 << (2 * 2 )));
	
	/*配置GPg3 11 为输入引脚*/
	*gpgcon  &= ~((0x03 << (3 * 2 )) |(0x03 << (11 * 2 )));
	return 0;
}
ssize_t second_drv_read(struct file *filp, char __user *buff,  size_t size, loff_t *ppos)
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
static ssize_t second_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	return 0;
}
static struct file_operations second_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =  second_drv_open,     
    .read	=   second_drv_read,	 //  .read	=   second_drv_read 如果将read	 错误写成write 这个实验会疯狂打印
};
int major;
static int second_drv_init(void)
{
	major = register_chrdev(0,"second_drv",&second_drv_fops);//返回主设备号
	
	second_class = class_create(THIS_MODULE, "second_drv");
	/*这样内核会自动mdev被内核调用根据sysfs下的信息创建buttons设备节点 */
	second_class_dev = class_device_create(second_class, NULL, MKDEV(major, 0), NULL, "buttons"); 
	gpfcon = (volatile unsigned long *)ioremap(0x56000050,16);
	gpfdat = gpfcon + 1;//因为0x56000054
	
	gpgcon = (volatile unsigned long *)ioremap(0x56000060,16);
	gpgdat = gpgcon + 1;//因为0x56000054
	return 0; 
}
static void second_drv_exit(void)
{
	unregister_chrdev(major,"second_drv");
	class_device_unregister(second_class_dev);
	class_destroy(second_class);
	iounmap(gpfcon);
	iounmap(gpgcon);
	return 0;
}
module_init(second_drv_init);//修饰入口函数
module_exit(second_drv_exit);//
MODULE_LICENSE("GPL");

