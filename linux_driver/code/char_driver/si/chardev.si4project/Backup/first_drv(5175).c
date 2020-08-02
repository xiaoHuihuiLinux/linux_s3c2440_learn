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
	/*����GPF 4 5 6Ϊ���*/
	*gpfcon &= ~( (0x3 << (4*2)) | (0x3 << (5*2)) | (0x3 << (6*2)));//led1 led2 led3
	*gpfcon |= ( (0x1 << (4*2)) | (0x1 << (5*2)) | (0x1 << (6*2)));//�Ƚ�������0����һ
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	//printk("first_drv_write\n");
	//wirte(fd,value,len)
	//�������ϲ����write()����ʱ��������εõ����ݴ��û��ռ䵽�ں˿ռ�
	//copy_from_user(); �ں˵��û��ռ� copy_to_user()������ʵ�����û��ռ䵽�ں˿ռ�����ݴ���
	int val;
	copy_from_user(&val,buf,count);//val�ǿ���������ֵ buf�Ǵ��ݵ�ֵcount�ǳ���
	if(1 == val )//
	{
		*gpfdat &= ~(1 << 4) | (1 << 5) | (1 << 6);
	}
	else 
	{
		
		*gpfdat |= (1 << 4) | (1 << 5) | (1 << 6);//����
	}
	
	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   first_drv_open,     
    .write	=	first_drv_write,	   
};
int major;
static int first_drv_init(void)
{
	//���� major, *name struct file_operations*fops major�����������0�����ʾ����ϵͳ��̬��������豸��
	register_chrdev(0, "first_drv", &first_drv_fops);// ע��, �����ں�
	firstdrv_class = class_create(THIS_MODULE, "firstdrv");//��һ������ָ��������������ĸ�ģ�飬�ڶ�������ָ�������� 
	//����ģ���ʱ���û��ռ��е�udev���Զ���Ӧdevice_create(��)������ȥ/sysfs��Ѱ�Ҷ�Ӧ����Ӷ������豸�ڵ㡣
	//��һ������ָ����Ҫ�������豸���������࣬�ڶ�������������豸�ĸ��豸�����û�о�ָ��ΪNULL���������������豸�ţ����ĸ��������豸���ƣ�����������Ǵ��豸�š�
	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); //��devĿ¼�´�����Ӧ���豸�ڵ�
	gpfcon = (volatile unsigned long*)ioremap(0x56000050,16);//ӳ�������ַ�������ַ 
	gpfdat = gpfcon+1;//data�ĵ�ַ��0x56000054�������Ļ����ϵ�ַ+1
	return 0;
}

static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv"); //  ж��
	class_device_unregister(firstdrv_class_dev);
	class_destroy(firstdrv_class);
	iounmap(gpfcon);
}

module_init(first_drv_init);//�ں���֪��ʲôʱ���������Ҫ����һ��
module_exit(first_drv_exit);//ͬ������Ҫ����

