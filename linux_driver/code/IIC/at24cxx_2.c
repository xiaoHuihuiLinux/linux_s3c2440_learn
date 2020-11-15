#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };//�����ַֻȡ����λ1010000 �����Ϊ��ĵ�ַ�򲻴���


/*
unsigned short **forces = address_data->forces;
forces[0][0] != I2C_CLIENT_END;
Դ������������
�������Ǽ���unsigned short * ΪAAA\
AAA *forces = address_data->forces //�������ǵ�forces ��AAA *forces  
AAA *forces  �������ʾ����AAA force[] ����.forces = unsigned short * force[]
����Ϊforces[0][0]�Ƕ�ά��������AAA force[]�еĵ�һ��Ԫ����Ȼ�Ǹ�����
*/
 
static unsigned short force_addr[] = {ANY_I2C_BUS,0x60,I2C_CLIENT_END};//���������ߵ��豸��ַ�������������˳�����
static unsigned short * forces[] = {force_addr,NULL};

static struct i2c_client_address_data addr_data = {
	.normal_i2c	= ignore/*normal_addr*/,/*Ҫ����s�źź��豸��ַ���õ�ack�źţ���ַ�źŲ���ȷ���Ƿ��������豸���ܵ��������at24cxx_detect*/
	.probe		= ignore,
	.ignore		= ignore,
	.forces = forces, /*ǿ����Ϊ��������豸*/
};
//������ڵ��豸�����õ�������ϣ���������������ĩ����
static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{
	printk("at24cxx is detch\n");//ֻ�Ǽ�һ����ӡ
	return 0;
}
static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);//����豸���ڵĻ��͵��õ���������
}
static int at24cxx_detach(struct i2c_client *client)
{
	printk("at24cxx is detch\n");//ֻ�Ǵ�ӡһ��

	return 0;
}

/* This is the driver that will be inserted */
static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name	= "at24cxx",
	},
	//.id		= I2C_DRIVERID_EEPROM,����Ҫ
	.attach_adapter	= at24cxx_attach,
	.detach_client	= at24cxx_detach,
};

static int at24cxx_init(void)
{
	i2c_add_driver(&at24cxx_driver);
	return 0;	
}
static void  at24cxx_exit(void)
{
	i2c_del_driver(&at24cxx_driver);
}
module_init(at24cxx_init);
module_exit(at24cxx_exit);
MODULE_LICENSE("GPL");

