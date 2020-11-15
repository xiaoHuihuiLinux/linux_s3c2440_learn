#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/fs.h>

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
static int major;
static struct class *cls;
static struct i2c_driver at24cxx_driver;//������
struct i2c_client *at24cxx_client;
static ssize_t at24cxx_read (struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}
static ssize_t at24cxx_write (struct file *file, const char __user *buf, size_t size, loff_t *offset)
{
	return 0;
}

static struct file_operations  at24cxx_fops = {
	.owner = THIS_MODULE,
	.read  = at24cxx_read,
	.write = at24cxx_write,
};
static struct i2c_client_address_data addr_data = {
	.normal_i2c	= ignore,/*normal_addr*//*Ҫ����s�źź��豸��ַ���õ�ack�źţ���ַ�źŲ���ȷ���Ƿ��������豸���ܵ��������at24cxx_detect*/
	.probe		= ignore,
	.ignore		= ignore,
	.forces = forces, /*ǿ����Ϊ��������豸*/
};

//������ڵ��豸�����õ�������ϣ���������������ĩ����Ҳ����ǿ��������0x60�������
static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{
	
	//����clinet�ṹ��
	at24cxx_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);//ע��i2c_client�ο���Դ����eeprom_data
	at24cxx_client->addr = address;
	at24cxx_client->adapter = adapter;//ָ��adapter
	at24cxx_client->driver = &at24cxx_driver;//ָ��driver�ṹ��
	at24cxx_client->flags = 0;
	/* Fill in the remaining client fields */
	strcpy(at24cxx_client->name, "at24cxx");//Դ�벻����strcpy
	i2c_attach_client(at24cxx_client);
	printk("at24cxx is atach\n");//ֻ�Ǽ�һ����ӡ

	major = register_chrdev(0, "at24cxx", &at24cxx_fops);//��һ��������0��ʾϵͳ�Զ�����
	
	cls = class_create(THIS_MODULE, "at24cxx");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "at24cxx"); /* /dev/at24cxx */ //�����豸�ڵ�
	return 0;
}
static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);//����豸���ڵĻ��͵��õ���������
}
//�����ǵ���ж�������ʱ��ΪʲĪû�д�ӡ���� ���ǲο�eeprom�еķ�����Ҫ���i2c_client
static int at24cxx_detach(struct i2c_client *client)
{
	printk("at24cxx_detach\n");
	i2c_detach_client(client);
	kfree(i2c_get_clientdata(client));
	
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "at24cxx");

	

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

