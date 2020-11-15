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
#include <linux/dma-mapping.h>

#define MEM_CPY_NO_DMA  0
#define MEM_CPY_DMA     1

#define BUF_SIZE  (512*1024)

#define DMA0_BASE_ADDR  0x4B000000
#define DMA1_BASE_ADDR  0x4B000040
#define DMA2_BASE_ADDR  0x4B000080
#define DMA3_BASE_ADDR  0x4B0000C0

struct s3c_dma_regs {
	unsigned long disrc;
	unsigned long disrcc;
	unsigned long didst;
	unsigned long didstc;
	unsigned long dcon;
	unsigned long dstat;
	unsigned long dcsrc;
	unsigned long dcdst;
	unsigned long dmasktrig;
};


static int major = 0;

static char *src;
static u32 src_phys;

static char *dst;
static u32 dst_phys;

static struct class *cls;

static volatile struct s3c_dma_regs *dma_regs;

static DECLARE_WAIT_QUEUE_HEAD(dma_waitq);
/* 中断事件标志, 中断服务程序将它置1，ioctl将它清0 */
static volatile int ev_dma = 0;

static int s3c_dma_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{	//接受来自应用的参数，来选择是用dma或者不用
	int i;
	memset(src,0xAA,BUF_SIZE);
	memset(dst,0x55,BUF_SIZE);
	switch(cmd)
	{
		case MEM_CPY_NO_DMA:
		{
			for(i = 0;i < BUF_SIZE;i++)
				dst[i] = src[i];
			if(memcmp(src,dst,BUF_SIZE) == 0)
			{
				printk("MEM_CPY_NO_DMA OK\n");//表示拷贝成功
			}
			else 
			{
				printk("MEM_CPY_DMA ERROR\n");//表示拷贝失败
			}
			break;
		}
		case MEM_CPY_DMA:
		{
			ev_dma = 0;//设置为0
			/* 把源,目的,长度告诉DMA */
			dma_regs->disrc      = src_phys;        /* 源的物理地址 */
			//源控制
			/*[1] 0: the source is in the system bus (AHB).1: the source is in the peripheral bus (APB).
			  [0] Bit 0 is used to select the address increment.源地址是递增的还是不变的我们都设置为0*/
			dma_regs->disrcc     = (0<<1) | (0<<0); /* 源位于AHB总线, 源地址递增 */
			dma_regs->didst      = dst_phys;        /* 目的的物理地址 */
			//目的控制
			/*[2]Select interrupt occurrence time when auto reload is setting.我们不要自动加载
			[1]Bit 1 is used to select the location of destination. 选择系统总线
			[0]Bit 0 is used to select the address increment. 地址是递增的
			*/
			dma_regs->didstc     = (0<<2) | (0<<1) | (0<<0); /* 目的位于AHB总线, 目的地址递增 */
			/*[31] 0: Demand mode will be selected.
			 [30]DREQ and DACK are synchronized to HCLK (AHB clock).选择的总线 不设置这个的话我们执行了程序之后就不会打印printk("MEM_CPY_DMA OK\n");
			 [29] 使能中断
			 TSZ[28]也就是单次传输或者burst一次传输的四个burst.还是一个单元:  unit transfer is performed.1: A burst transfer of length four is performed.
			 [27]Single service传输完后等待下一次dma请求  whole service。DMA会一直占用住总线，其他介质不能用 需要设置1
			 [26-24]DMA request source for each DMA. DMA 请求源我们没有用到我们是用软件模拟的
			 [23]0: S/W request mode is selected and DMA is triggered by setting 设置为0用软件触发
			 [22]Set the reload on/off option. 从新加载
			 DSZ[21 -20] Data size to be transferred. 一个字节或者四个字节 读写数据的大小
			 TC[19:0]Initial transfer count (or transfer beat). 传输的count
			 总容量 = TC *TSZ（1 或者4）*DSZ
			 */
			dma_regs->dcon       = (1<<30)|(1<<29)|(0<<28)|(1<<27)|(0<<23)|(0<<20)|(BUF_SIZE<<0);  /* 使能中断,单个传输,软件触发, */
			//DSTAT 状态寄存器我们暂时不管 
			/* 启动DMA */
			/*DMASKTRIG
			[2]DMA stops as soon as the current atomic transfer ends立刻停止dm1
			[1]ON_OFF DMA channel is turned on and  打开dm1
			[0]SW_TRIG requests a DMA operation to this controller 软件触发
			*/
			dma_regs->dmasktrig  = (1<<1) | (1<<0);
			/* 如何知道DMA什么时候完成?传输完有一个中断 */
			/* 启动DMA -》休眠 DMA  DMA完成后唤醒 */
			wait_event_interruptible(dma_waitq, ev_dma);//ev_dma =0所以休眠
			//只有在中断s3c_dma_irq将他唤醒后才能继续往下执行 [30] [27]需要设置为1
			if (memcmp(src, dst, BUF_SIZE) == 0)
			{
				printk("MEM_CPY_DMA OK\n");/*复制完成*/
			}
			else
			{
				printk("MEM_CPY_DMA ERROR\n");
			}
			break;
		}
		default:
			break;
	}

	return 0;
}

static struct file_operations  dma_fops= {
	.owner  = THIS_MODULE,
	.ioctl  = s3c_dma_ioctl,
};

static irqreturn_t s3c_dma_irq(int irq, void *devid)
{
	ev_dma = 1;/* 唤醒 */
    wake_up_interruptible(&dma_waitq);   /* 唤醒休眠的进程 */
	return IRQ_HANDLED;
}

static int s3c_dma_init(void)
{
	//注册中断
	//再开发板上要查看 cat /proc/interrupts 因为我们的前几个都被用了所以我们IRQ_DMA3 36 
	if(request_irq(IRQ_DMA3,s3c_dma_irq,0,"s3c_dma",1) )//第一个参数要考虑有无用这个中断号
	{
		printk("can't request_irq for DMA\n");
		return -EBUSY;
	}
	/*1.分配src dst 对应的缓冲区，因为我们用内存模拟*/
	//不能用kmalloc因为它虽然虚拟地址是连续的但是对应的物理地址不连续
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL)
	{
		printk("can't alloc buffer for src\n");
		free_irq(IRQ_DMA3, 1);//申请不成功的话删除中断
		return -ENOMEM;
	}
	dst = dma_alloc_writecombine(NULL,BUF_SIZE,&dst_phys,GFP_KERNEL);
	if(dst == NULL)
	{
		//如果申请的dst失败则将src 释放
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		free_irq(IRQ_DMA3, 1);
		printk("can't alloc buffer for dst\n");
		return -ENOMEM;
	}
	//注册并创建设备节点
	major = register_chrdev(0,"s3c_dma",&dma_fops);
	cls = class_create(THIS_MODULE,"s3c_dma");
	class_device_create(cls,NULL,MKDEV(major, 0),NULL,"dma");//dev/dma
	/*地址映射,我们要映射DMA3_BASE_ADDR*/
	dma_regs = ioremap(DMA3_BASE_ADDR,sizeof(struct s3c_dma_regs));
	return 0;	
}
static void s3c_dma_exit(void)
{
	iounmap(dma_regs);
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "s3c_dma");
	dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
	dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);	
	free_irq(IRQ_DMA3, 1);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);

MODULE_LICENSE("GPL");


