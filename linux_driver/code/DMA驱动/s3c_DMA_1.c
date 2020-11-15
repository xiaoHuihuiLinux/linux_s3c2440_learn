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
{	//接受来自应用的参数，来选择是用dma或者不用
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
	/*1.分配src dst 对应的缓冲区，因为我们用内存模拟*/
	//不能用kmalloc因为它虽然虚拟地址是连续的但是对应的物理地址不连续
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL)
	{
		printk("can't alloc buffer for src\n");
		return -ENOMEM;
	}
	dst = dma_alloc_writecombine(NULL,BUF_SIZE,&dst_phys,GFP_KERNEL);
	if(dst == NULL)
	{
		//如果申请的dst失败则将src 释放
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		printk("can't alloc buffer for dst\n");
		return -ENOMEM;
	}
	//注册并创建设备节点
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


