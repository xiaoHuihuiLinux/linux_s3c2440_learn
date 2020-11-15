/*
可以参考这两个文件的写法
drivers\mtd\nand\at91_nand.c
drivers\mtd\nand\s3c2410.c
*/
//#include <linux/slab.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
 
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
 
#include <asm/io.h>
 
#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>

struct s3c_nand_regs {
	unsigned long nfconf  ;
	unsigned long nfcont  ;
	unsigned long nfcmd   ;
	unsigned long nfaddr  ;
	unsigned long nfdata  ;
	unsigned long nfeccd0 ;
	unsigned long nfeccd1 ;
	unsigned long nfeccd  ;
	unsigned long nfstat  ;
	unsigned long nfestat0;
	unsigned long nfestat1;
	unsigned long nfmecc0 ;
	unsigned long nfmecc1 ;
	unsigned long nfsecc  ;
	unsigned long nfsblk  ;
	unsigned long nfeblk  ;
};

static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;//nand_scan(mtd, 1)/nand_scan第一个参数位mtd
static struct s3c_nand_regs* s3c_nand_regs;
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{	//取消选中
		s3c_nand_regs->nfcont |=  (1 << 1) ;
	}
	else 
	{	//选中片选NFCONT[1]设为1
		//0: Force nFCE to low (Enable chip select)
		//1: Force nFCE to high (Disable chip select)
		s3c_nand_regs->nfcont &= ~(1 << 1);
	}
}
static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{	//发命令 将cmd = NFCMMD
		s3c_nand_regs->nfcmd = cmd;
	}
	else
	{	//发地址cmd = NFADDR
		s3c_nand_regs->nfaddr = cmd;
	}
}
static int	s3c2440_dev_ready(struct mtd_info *mtd)
{	//状态寄存器的bit0
	return (s3c_nand_regs->nfstat & (1 << 0));
}

static int s3c_nand_init(void)
{
	struct clk *clk;
	//1分配nand_chip结构体
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	
	s3c_nand_regs = ioremap(0x4E000000,sizeof(struct s3c_nand_regs));
	//2设置nand_chip
	//设置nand_chip是给nand_scan函数使用
	//发命令 发地址 发数据 读数据 读状态
	s3c_nand->select_chip = s3c2440_select_chip;//提供的默认的不适用
	s3c_nand->cmd_ctrl = s3c2440_cmd_ctrl;//因为默认的函数最后也是调用cmd_ctrl函数
	s3c_nand->IO_ADDR_R = &s3c_nand_regs->nfdata;//读 因为提供的默认的函数最终也是指向这个地址
	s3c_nand->IO_ADDR_W = &s3c_nand_regs->nfdata;//写 因为提供的默认的函数最终也是指向这个地址
	s3c_nand->dev_ready = s3c2440_dev_ready;
	//NAND_ECC_NONE selected by board driver. This is not recommended !!
	/*如果不加上ecc校验的话会提示*/
	s3c_nand->ecc.mode = NAND_ECC_SOFT;	/* enable ECC */
	//3硬件相关的操作
	//使能nand_flsh控制器时钟总开关
	clk = clk_get(NULL, "nand");
	clk_enable(clk);//实际上就是CLKCON的bit4
	/*一些时序的设置也就是时间参数：在NFCONF寄存器*/
	/*在nand芯片手册中 twp 最小 12ns //tcls最小12ns*/// tclh最小5ns 所以tacls可以设置为0
	//HCLK = 100M 所以为10ns
	//TACLS:发出ALE/CLE多久才能发出nWE信号,从nand手册可知ALE/CLE可以和nWE同时发出
	//TWRPH0 nWE的宽度 HCLK x(TWRPH0 +1) 所以TWRPH0 >=1
	//TWRPH1 nWE变为高电平后多长时间可以变为低电平HCLK x(TWRPH1 +1)所以TWRPH1>=0
	#define TACLS 0
	#define TWRPH0 1
	#define TWRPH1 0
	s3c_nand_regs->nfconf = (TACLS << 12 ) | (TWRPH0 << 8) | (TWRPH1 << 4);
	/*取消片选
	使能nand_flash控制器*/
	s3c_nand_regs->nfcont = (1 << 1) | (1<< 0);
	
	//4使用它nand_scan()
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	s3c_mtd->priv = s3c_nand;//mtd的私有数据等于nand_chip结构体。将mtd结构体和nand_chip结构体联系起来
	s3c_mtd->owner = THIS_MODULE;
	nand_scan(s3c_mtd, 1);//参数2位芯片的个数为1.nand_scan扫描，识别，构造mtd_info结构体（有读写擦除）
	//5 add_mtd_paritions
	return 0;	
}
static void s3c_nand_exit(void)
{
	kfree(s3c_mtd);
	iounmap(s3c_nand_regs);
	kfree(s3c_nand);
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");

