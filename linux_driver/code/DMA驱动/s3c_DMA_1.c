#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/poll.h>

#define BUF_SIZE  (512*1024)//512k

#define MEM_CPY_NO_DMA  0
#define MEM_CPY_DMA     1

static char*src;
static u32 src_phys;
static char*dst;
static u32 dst_phys;

static struct class  *cls;
static int s3c_dma_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{	//��������Ӧ�õĲ�������ѡ������dma���߲���
	switch()
	{
		case MEM_CPY_NO_DMA:
		{
			break;
		}
		case MEM_CPY_DMA:
		{
			break;
		}
		default:
			break;
	}

	return 0;
}

static struct file_operations  dam_fops= {
	.owner  = THIS_MODULE,
	.ioctl  = s3c_dma_ioctl,
};
int major;
static int s3c_dma_init(void)
{
	/*1.����src dst ��Ӧ�Ļ���������Ϊ�������ڴ�ģ��*/
	//������kmalloc��Ϊ����Ȼ�����ַ�������ĵ��Ƕ�Ӧ�������ַ������
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL)
	{
		printk("can't alloc buffer for src\n");
		return -ENOMEM;
	}
	dst = dma_alloc_writecombine(NULL,BUF_SIZE,&dst_phys,GFP_KERNEL);
	if(dst == NULL)
	{
		//��������dstʧ����src �ͷ�
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		printk("can't alloc buffer for dst\n");
		return -ENOMEM;
	}
	//ע�Ტ�����豸�ڵ�
	major = register_chrdev(0,"s3c_dma",&dam_fops);
	cls = class_create(THIS_MODULE,"s3c_dma");
	class_device_create(cls,NULL,MKDEV(major, 0),NULL,"dma");//dev/dma
	
	return 0;	
}
static void s3c_dma_exit(void)
{
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major,"s3c_dma");
	dma_alloc_writecombine(NULL,BUF_SIZE,src,src_phys);
	dma_alloc_writecombine(NULL,BUF_SIZE,dst,dst_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);

MODULE_LICENSE("GPL");


