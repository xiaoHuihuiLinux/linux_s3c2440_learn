#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/fs.h>

static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };//这个地址只取高七位1010000 如果改为别的地址则不存在


/*
unsigned short **forces = address_data->forces;
forces[0][0] != I2C_CLIENT_END;
源码中是这样的
所以我们假如unsigned short * 为AAA\
AAA *forces = address_data->forces //所以我们的forces 是AAA *forces  
AAA *forces  用数组表示就是AAA force[] 所以.forces = unsigned short * force[]
又因为forces[0][0]是二维数组所以AAA force[]中的第一个元素依然是个数组
*/
 
static unsigned short force_addr[] = {ANY_I2C_BUS,0x60,I2C_CLIENT_END};//在呢条总线的设备地址第三个参数是退出条件
static unsigned short * forces[] = {force_addr,NULL};
static int major;
static struct class *cls;
static struct i2c_driver at24cxx_driver;//声明下
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
	.normal_i2c	= ignore,/*normal_addr*//*要发出s信号和设备地址并得到ack信号，地址信号才能确定是否存在这个设备才能调这个函数at24cxx_detect*/
	.probe		= ignore,
	.ignore		= ignore,
	.forces = forces, /*强制认为存在这个设备*/
};

//如果现在的设备不能用但是我们希望调用这个函数怎末办呢也就是强制任务有0x60这个驱动
static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{
	
	//构造clinet结构体
	at24cxx_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);//注意i2c_client参考的源码是eeprom_data
	at24cxx_client->addr = address;
	at24cxx_client->adapter = adapter;//指向adapter
	at24cxx_client->driver = &at24cxx_driver;//指向driver结构体
	at24cxx_client->flags = 0;
	/* Fill in the remaining client fields */
	strcpy(at24cxx_client->name, "at24cxx");//源码不是用strcpy
	i2c_attach_client(at24cxx_client);
	printk("at24cxx is atach\n");//只是加一个打印

	major = register_chrdev(0, "at24cxx", &at24cxx_fops);//第一个参数是0表示系统自动分配
	
	cls = class_create(THIS_MODULE, "at24cxx");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "at24cxx"); /* /dev/at24cxx */ //构造设备节点
	return 0;
}
static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);//如果设备存在的话就调用第三个函数
}
//在我们调用卸载命令的时候为什莫没有打印出来 我们参考eeprom中的发现需要添加i2c_client
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
	//.id		= I2C_DRIVERID_EEPROM,不重要
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

