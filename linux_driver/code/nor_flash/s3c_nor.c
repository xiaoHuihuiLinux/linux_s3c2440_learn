/*具体可以参考内核提供的驱动：
D:\WeiDongShan_Learn\内核配置裁剪以及启动流程\linux-2.6.22.6_Jz2440_patch\linux-2.6.22.6\drivers\mtd\maps\physmap.c
*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <asm/io.h>

static struct map_info *s3c_nor_map;//在我们参考的内核代码physmap_flash_probe()分配的是physmap_flash_info
static struct mtd_info	*s3c_nor_mtd;
static struct mtd_partition s3c_nor_parts[] = {
	[0] = {
        .name   = "bootloader_nor",
        .size   = 0x00040000,
		.offset	= 0,
	},
	[1] = {
        .name   = "root_nor",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,
	}
};

static int s3c_nor_init(void)
{
	/*我们要做的就是操作硬件的部分：*/
	//1.分配map_info
	s3c_nor_map = kzalloc(sizeof(struct map_info), GFP_KERNEL);
	//2.设置物理基地址(phys), 大小(size), 位宽(bankwidth), 虚拟基地址(virt)这是每个nor差异的地方
	s3c_nor_map->name = "s3c_nor";//名字
	s3c_nor_map->phys = 0;//物理地址0
	s3c_nor_map->size = 0x1000000;//大于nor真实地址
	s3c_nor_map->bankwidth = 2;//位宽16
	s3c_nor_map->virt =  ioremap(s3c_nor_map->phys, s3c_nor_map->size);//虚拟地址

	simple_map_init(s3c_nor_map);
	// 3.使用用NOR FLASH 协议层提供的函数来识别
	printk("use cfi_probe\n");
	s3c_nor_mtd = do_map_probe("cfi_probe", s3c_nor_map);
	if(!s3c_nor_mtd)//不是cfi类型的话其实我们就是用cfi
	{
		printk("use jedec_probe\n");
		s3c_nor_mtd = do_map_probe("jedec_probe", s3c_nor_map);
	}
	if(!s3c_nor_mtd)
	{
		iounmap(s3c_nor_map->virt);
		kfree(s3c_nor_map);
		return -EIO;
	}
	// add_mtd_partitions 加上分区信息
	add_mtd_partitions(s3c_nor_mtd,s3c_nor_parts,2);
	
	return 0;
}
static void s3c_nor_exit(void)
{
	del_mtd_partitions(s3c_nor_mtd); //清除分区结构数组.
	iounmap(s3c_nor_map->virt);
	kfree(s3c_nor_map);
	return;
}
module_init(s3c_nor_init);
module_exit(s3c_nor_exit);

MODULE_LICENSE("GPL");


