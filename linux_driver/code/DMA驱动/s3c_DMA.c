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
/* �ж��¼���־, �жϷ����������1��ioctl������0 */
static volatile int ev_dma = 0;

static int s3c_dma_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{	//��������Ӧ�õĲ�������ѡ������dma���߲���
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
				printk("MEM_CPY_NO_DMA OK\n");//��ʾ�����ɹ�
			}
			else 
			{
				printk("MEM_CPY_DMA ERROR\n");//��ʾ����ʧ��
			}
			break;
		}
		case MEM_CPY_DMA:
		{
			ev_dma = 0;//����Ϊ0
			/* ��Դ,Ŀ��,���ȸ���DMA */
			dma_regs->disrc      = src_phys;        /* Դ�������ַ */
			//Դ����
			/*[1] 0: the source is in the system bus (AHB).1: the source is in the peripheral bus (APB).
			  [0] Bit 0 is used to select the address increment.Դ��ַ�ǵ����Ļ��ǲ�������Ƕ�����Ϊ0*/
			dma_regs->disrcc     = (0<<1) | (0<<0); /* Դλ��AHB����, Դ��ַ���� */
			dma_regs->didst      = dst_phys;        /* Ŀ�ĵ������ַ */
			//Ŀ�Ŀ���
			/*[2]Select interrupt occurrence time when auto reload is setting.���ǲ�Ҫ�Զ�����
			[1]Bit 1 is used to select the location of destination. ѡ��ϵͳ����
			[0]Bit 0 is used to select the address increment. ��ַ�ǵ�����
			*/
			dma_regs->didstc     = (0<<2) | (0<<1) | (0<<0); /* Ŀ��λ��AHB����, Ŀ�ĵ�ַ���� */
			/*[31] 0: Demand mode will be selected.
			 [30]DREQ and DACK are synchronized to HCLK (AHB clock).ѡ������� ����������Ļ�����ִ���˳���֮��Ͳ����ӡprintk("MEM_CPY_DMA OK\n");
			 [29] ʹ���ж�
			 TSZ[28]Ҳ���ǵ��δ������burstһ�δ�����ĸ�burst.����һ����Ԫ:  unit transfer is performed.1: A burst transfer of length four is performed.
			 [27]Single service�������ȴ���һ��dma����  whole service��DMA��һֱռ��ס���ߣ��������ʲ����� ��Ҫ����1
			 [26-24]DMA request source for each DMA. DMA ����Դ����û���õ������������ģ���
			 [23]0: S/W request mode is selected and DMA is triggered by setting ����Ϊ0���������
			 [22]Set the reload on/off option. ���¼���
			 DSZ[21 -20] Data size to be transferred. һ���ֽڻ����ĸ��ֽ� ��д���ݵĴ�С
			 TC[19:0]Initial transfer count (or transfer beat). �����count
			 ������ = TC *TSZ��1 ����4��*DSZ
			 */
			dma_regs->dcon       = (1<<30)|(1<<29)|(0<<28)|(1<<27)|(0<<23)|(0<<20)|(BUF_SIZE<<0);  /* ʹ���ж�,��������,�������, */
			//DSTAT ״̬�Ĵ���������ʱ���� 
			/* ����DMA */
			/*DMASKTRIG
			[2]DMA stops as soon as the current atomic transfer ends����ֹͣdm1
			[1]ON_OFF DMA channel is turned on and  ��dm1
			[0]SW_TRIG requests a DMA operation to this controller �������
			*/
			dma_regs->dmasktrig  = (1<<1) | (1<<0);
			/* ���֪��DMAʲôʱ�����?��������һ���ж� */
			/* ����DMA -������ DMA  DMA��ɺ��� */
			wait_event_interruptible(dma_waitq, ev_dma);//ev_dma =0��������
			//ֻ�����ж�s3c_dma_irq�������Ѻ���ܼ�������ִ�� [30] [27]��Ҫ����Ϊ1
			if (memcmp(src, dst, BUF_SIZE) == 0)
			{
				printk("MEM_CPY_DMA OK\n");/*�������*/
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
	ev_dma = 1;/* ���� */
    wake_up_interruptible(&dma_waitq);   /* �������ߵĽ��� */
	return IRQ_HANDLED;
}

static int s3c_dma_init(void)
{
	//ע���ж�
	//�ٿ�������Ҫ�鿴 cat /proc/interrupts ��Ϊ���ǵ�ǰ��������������������IRQ_DMA3 36 
	if(request_irq(IRQ_DMA3,s3c_dma_irq,0,"s3c_dma",1) )//��һ������Ҫ��������������жϺ�
	{
		printk("can't request_irq for DMA\n");
		return -EBUSY;
	}
	/*1.����src dst ��Ӧ�Ļ���������Ϊ�������ڴ�ģ��*/
	//������kmalloc��Ϊ����Ȼ�����ַ�������ĵ��Ƕ�Ӧ�������ַ������
	src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
	if(src == NULL)
	{
		printk("can't alloc buffer for src\n");
		free_irq(IRQ_DMA3, 1);//���벻�ɹ��Ļ�ɾ���ж�
		return -ENOMEM;
	}
	dst = dma_alloc_writecombine(NULL,BUF_SIZE,&dst_phys,GFP_KERNEL);
	if(dst == NULL)
	{
		//��������dstʧ����src �ͷ�
		dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
		free_irq(IRQ_DMA3, 1);
		printk("can't alloc buffer for dst\n");
		return -ENOMEM;
	}
	//ע�Ტ�����豸�ڵ�
	major = register_chrdev(0,"s3c_dma",&dma_fops);
	cls = class_create(THIS_MODULE,"s3c_dma");
	class_device_create(cls,NULL,MKDEV(major, 0),NULL,"dma");//dev/dma
	/*��ַӳ��,����Ҫӳ��DMA3_BASE_ADDR*/
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


