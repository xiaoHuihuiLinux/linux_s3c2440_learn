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

static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_dev;


volatile unsigned long* gpfcon = NULL;
volatile unsigned long* gpfdat = NULL;

static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/*配置GPF 4 5 6为输出*/
	*gpfcon &= ~( (0x3 << (4*2)) | (0x3 << (5*2)) | (0x3 << (6*2)));//led1 led2 led3
	*gpfcon |= ( (0x1 << (4*2)) | (0x1 << (5*2)) | (0x1 << (6*2)));//先将整体清0再置一
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	//printk("first_drv_write\n");
	//wirte(fd,value,len)
	//当我们上层调用write()函数时，我们如何得到数据从用户空间到内核空间
	//copy_from_user(); 内核到用户空间 copy_to_user()这样就实现了用户空间到内核空间的数据传递
	int val;
	copy_from_user(&val,buf,count);//val是拷贝过来的值 buf是传递的值count是长度
	if(1 == val )//
	{
		*gpfdat &= ~(1 << 4) | (1 << 5) | (1 << 6);
	}
	else 
	{
		
		*gpfdat |= (1 << 4) | (1 << 5) | (1 << 6);//亮灯
	}
	
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,     
    .write	=	first_drv_write,	   
};
int major;
static int first_drv_init(void)
{
	//参数 major, *name struct file_operations*fops major参数如果等于0，则表示采用系统动态分配的主设备号
	register_chrdev(0, "first_drv", &first_drv_fops);// 注册, 告诉内核
	firstdrv_class = class_create(THIS_MODULE, "firstdrv");//第一个参数指定类的所有者是哪个模块，第二个参数指定类名。 
	//加载模块的时候，用户空间中的udev会自动响应device_create(…)函数，去/sysfs下寻找对应的类从而创建设备节点。
	//第一个参数指定所要创建的设备所从属的类，第二个参数是这个设备的父设备，如果没有就指定为NULL，第三个参数是设备号，第四个参数是设备名称，第五个参数是从设备号。
	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); //在dev目录下创建相应的设备节点
	gpfcon = (volatile unsigned long*)ioremap(0x56000050,16);//映射物理地址到虚拟地址 
	gpfdat = gpfcon+1;//data的地址是0x56000054在上述的基础上地址+1
	return 0;
}

static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv"); //  卸载
	class_device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpfcon);
}

module_init(first_drv_init);//内核需知道什么时候调用所以要修饰一下
module_exit(first_drv_exit);//同样的需要修饰

