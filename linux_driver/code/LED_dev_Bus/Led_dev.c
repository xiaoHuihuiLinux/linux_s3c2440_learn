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
ģ�£����Ǵ��������ġ�linux-2.6.22.6\arch\arm\mach-s3c2440\Mach-smdk2440.c"�е�
���롣
static struct platform_device s3c2440_device_sdi
*/
/*��������ֻ����һ���ƣ����÷���ֲ㡪����������*/
/*���� ���� ע��һ��platform_device */
static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,//gpfcon�Ĵ�����ַ
        .end   = 0x56000050 + 8 - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = 4,//��һλ
        .end   = 4,
        .flags = IORESOURCE_IRQ,
    }

};
//void	(*release)(struct device * dev);
static void led_release (struct device * dev)
{
}
//platform_device������release�����ǵ�Ҫ����
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
device_add()�����˽���devcie���ṹ�ŵ�bus �ġ�dev ����֮�⣬�������һ�ߵġ�drv��
������ȡ��Ԫ��ĳ����driver���ṹ�����������һ����.match�����������Ƚϣ�����һ�ߵ�
��driver���Ƿ�֧��һ�ߵġ�device���������ܹ�֧�֣�����ŵ�������������ֵġ�.probe��
������

*/
static int led_dev_init(void)
{
	platform_device_register(&led_dev);//�Ὣbus_drv_devģ���е�Ӳ������device�ṹ��ŵ��������ߵ�ĳ��dev������
	return 0;
}
static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev);
}
module_init(led_dev_init);//�������������ǲ��ܱ�����
module_exit(led_dev_exit);
MODULE_LICENSE("GPL");


