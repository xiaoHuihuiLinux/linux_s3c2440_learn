/*
可以参考这两个文件的写法
drivers\mtd\nand\at91_nand.c
drivers\mtd\nand\s3c2410.c
*/
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
	
#include <asm/io.h>
#include <asm/sizes.h>
	
#include <asm/hardware.h>
#include <asm/arch/board.h>
#include <asm/arch/gpio.h>

static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;//nand_scan(mtd, 1)/nand_scan第一个参数位mtd
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{	//取消选中
	}
	else 
	{//选中片选NFCONT[1]设为1
	}
}
static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{	//发命令 将cmd = NFCMMD
	
	}
	else
	{	//发地址cmd = NFADDR
		
	}
}
static int	s3c2440_dev_ready(struct mtd_info *mtd);
{
	return NFSTATE[0]
}

static int s3c_nand_init(void)
{
	//1分配nand_chip结构体
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	//2设置nand_chip
	//设置nand_chip是给nand_scan函数使用
	//发命令 发地址 发数据 读数据 读状态
	s3c_nand->select_chip = s3c2440_select_chip();//提供的默认的不适用
	s3c_nand->cmd_ctrl = s3c2440_cmd_ctrl();//因为默认的函数最后也是调用cmd_ctrl函数
	s3c_nand->IO_ADDR_R = "NFDATA虚拟地址" ;//读 因为提供的默认的函数最终也是指向这个地址
	s3c_nand->IO_ADDR_W = "NFDATA虚拟地址" ;//写 因为提供的默认的函数最终也是指向这个地址
	s3c_nand->dev_ready = s3c2440_dev_ready();
	//3硬件相关的操作
	
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
	
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");

