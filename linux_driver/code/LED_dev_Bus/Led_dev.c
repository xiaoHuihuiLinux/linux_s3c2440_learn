#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/regs-gpio.h>
/*
模仿：这是打过补丁后的“linux-2.6.22.6\arch\arm\mach-s3c2440\Mach-smdk2440.c"中的
代码。
static struct platform_device s3c2440_device_sdi
*/
/*现在我们只点亮一个灯，采用分离分层——总线驱动*/
/*分配 配置 注册一个platform_device */
static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,//gpfcon寄存器地址
        .end   = 0x56000050 + 8 - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = 4,//哪一位
        .end   = 4,
        .flags = IORESOURCE_IRQ,
    }

};
//void	(*release)(struct device * dev);
static void led_release (struct device * dev)
{
}
//platform_device里面有release函数记得要加上
static struct platform_device led_dev = {
    .name         = "myled",
    .id       = -1,
    .num_resources    = ARRAY_SIZE(led_resource),
    .resource     = led_resource,
    .dev = {
    	.release = led_release,
	},
};
/*
device_add()”除了将“devcie”结构放到bus 的“dev 链表”之外，还会从另一边的“drv”
链表中取表元即某个“driver”结构，用总线里的一个（.match）函数来作比较，看另一边的
“driver”是否支持一边的“device”。若是能够支持，则接着调用软件驱动部分的“.probe”
函数。

*/
static int led_dev_init(void)
{
	platform_device_register(&led_dev);//会将bus_drv_dev模型中的硬件部分device结构体放到虚拟总线的某个dev链表中
	return 0;
}
static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev);
}
module_init(led_dev_init);//驱动中修饰他们才能被调用
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");


