#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };//这个地址只取高七位1010000

static struct i2c_client_address_data addr_data = {
	.normal_i2c	= normal_addr,/*要发出s信号和设备地址并得到ack信号，地址信号才能确定是否存在这个设备才能调这个函数at24cxx_detect*/
	.probe		= ignore,
	.ignore		= ignore,
	//.force = xxxxx /*强制认为存在这个设备*/
};
//如果现在的设备不能用但是我们希望调用这个函数怎末办呢
static int at24cxx_detect(struct i2c_adapter *adapter, int address, int kind)
{
	printk("at24cxx is detch\n");//只是加一个打印
	return 0;
}
static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);//如果设备存在的话就调用第三个函数
}
static int at24cxx_detach(struct i2c_client *client)
{
	printk("at24cxx is detch\n");//只是打印一下

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

