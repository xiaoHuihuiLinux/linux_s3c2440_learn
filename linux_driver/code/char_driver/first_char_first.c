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


static int first_drv_open(struct inode *inode, struct file *file)
{
	printk("first_drv_open\n");
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	printk("first_drv_write\n");
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
    .write	=	first_drv_write,	   
};
static int first_drv_init(void)
{
	register_chrdev(111, "first_drv", &first_drv_fops); // 注册, 告诉内核
	return 0;
}

static void first_drv_exit(void)
{
	unregister_chrdev(111, "first_drv"); // 卸载
}

module_init(first_drv_init);//修饰一下
module_exit(first_drv_exit);//修饰一下也就是注册函数指针



