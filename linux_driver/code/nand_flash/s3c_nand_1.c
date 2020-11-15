/*
���Բο��������ļ���д��
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
static struct mtd_info *s3c_mtd;//nand_scan(mtd, 1)/nand_scan��һ������λmtd
static void s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if(chipnr == -1)
	{	//ȡ��ѡ��
	}
	else 
	{//ѡ��ƬѡNFCONT[1]��Ϊ1
	}
}
static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{	//������ ��cmd = NFCMMD
	
	}
	else
	{	//����ַcmd = NFADDR
		
	}
}
static int	s3c2440_dev_ready(struct mtd_info *mtd);
{
	return NFSTATE[0]
}

static int s3c_nand_init(void)
{
	//1����nand_chip�ṹ��
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	//2����nand_chip
	//����nand_chip�Ǹ�nand_scan����ʹ��
	//������ ����ַ ������ ������ ��״̬
	s3c_nand->select_chip = s3c2440_select_chip();//�ṩ��Ĭ�ϵĲ�����
	s3c_nand->cmd_ctrl = s3c2440_cmd_ctrl();//��ΪĬ�ϵĺ������Ҳ�ǵ���cmd_ctrl����
	s3c_nand->IO_ADDR_R = "NFDATA�����ַ" ;//�� ��Ϊ�ṩ��Ĭ�ϵĺ�������Ҳ��ָ�������ַ
	s3c_nand->IO_ADDR_W = "NFDATA�����ַ" ;//д ��Ϊ�ṩ��Ĭ�ϵĺ�������Ҳ��ָ�������ַ
	s3c_nand->dev_ready = s3c2440_dev_ready();
	//3Ӳ����صĲ���
	
	//4ʹ����nand_scan()
	s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	s3c_mtd->priv = s3c_nand;//mtd��˽�����ݵ���nand_chip�ṹ�塣��mtd�ṹ���nand_chip�ṹ����ϵ����
	s3c_mtd->owner = THIS_MODULE;
	nand_scan(s3c_mtd, 1);//����2λоƬ�ĸ���Ϊ1.nand_scanɨ�裬ʶ�𣬹���mtd_info�ṹ�壨�ж�д������
	//5 add_mtd_paritions
	return 0;	
}
static void s3c_nand_exit(void)
{
	
}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");

