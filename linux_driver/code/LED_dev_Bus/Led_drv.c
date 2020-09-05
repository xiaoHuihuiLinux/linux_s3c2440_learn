

/* ·???/éè??/×￠2áò???platform_driver */

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

static int major;


static struct class *cls;
static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;


static int led_open(struct inode *inode, struct file *file)//open操作设置为输出
{

	/*配置GPF 4 5 6为输出引脚*/
	*gpio_con &= ~(0x3 << (pin*2));//led1 led2 led3
	*gpio_con |= (0x1 << (pin*2));//先将整体清0再置一
	return 0;
}
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	//wirte(fd,value,len)
	//当我们上层调用write()函数时，我们如何得到数据从用户空间到内核空间
	//copy_from_user(); 内核到用户空间 copy_to_user()这样就实现了用户空间到内核空间的数据传递
	int val;
	copy_from_user(&val,buf,count);//val是拷贝过来的值 buf是传递的值count是长度
	if(1 == val )//
	{
		*gpio_dat &= ~(1 << pin);//亮灯
	}
	else 
	{
		*gpio_dat |= (1 << pin) ;
	}
	return 0;
}

static struct file_operations led_fops = {
	.owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   led_open,     
    .write	=	led_write,	
};

static int  led_probe(struct platform_device *pdev)
{
	/*根据platform_device资源进行ioremap*/
	struct resource *res;//定义资源
	res = platform_get_resource(pdev,IORESOURCE_MEM,0);//IORESOURCE_MEM内存类资源
	gpio_con = ioremap(res->start,res->end - res->start + 1);
	gpio_dat = gpio_con + 1;//指针加1就相当于加4字节，就指向了gpio_dat寄存器了
	res = platform_get_resource(pdev,IORESOURCE_IRQ,0);//,获得IORESOURCE_IRQ中断资源。0表示这类IORESOURCE_IRQ资源里的第0个.
	pin = res->start;
	/*注册设备驱动*/
	printk("led_probe, found led\n");
	major = register_chrdev(0,"myled",&led_fops);
	cls = class_create(THIS_MODULE, "myled");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */
	return 0;
}
static int  led_remove(struct platform_device *pdev)
{
	/* D???×?·?éè±??y?ˉ3ìDò */
	/* iounmap */
	printk("led_remove, remove led\n");

	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);
	return 0;
}
struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",//跟device的name保持一致
	}
};
/*
driver_register()"会将“bus_drv_dev”模型中的较稳定代码“driver”结构体放到虚拟总线的
某个链表（drv 链表）中。从另一边的“dev”链表中取出每一个“device”结构用bus 中的
“.match”函数来作比较，若支持则调用“.probe”函数。
*/

static int led_drv_init(void)
{
	
	platform_driver_register(&led_drv);//向上注册一个driver.将bus_drv_dev模型中较稳定的代码driver放到虚拟总线的drv链表中
	
	return 0;
}
static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);

}
module_init(led_drv_init);//驱动中修饰他们才能被调用
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");


