#include "i2c_controller.h" 
#define I2C_CONTROLLER_NUM 10
/* 有一个i2c_controller数组用来存放各种不同芯片的操作结构体 */
static p_i2c_controller p_i2c_controllers[I2C_CONTROLLER_NUM];
static p_i2c_controller p_i2c_con_selected;
void register_i2c_controller(p_i2c_controller *p)
{
	int i;
	for(i = 0; i < I2C_CONTROLLER_NUM;i++)
	{
		if(!p_i2c_controllers[i])
		{
			p_i2c_controllers[i] = p;
			return;
		}
	}
}
/* 根据名字来选择某款I2C控制器 */
int select_i2c_controller(char *name)
{
	int i;
	for(i = 0; i < I2C_CONTROLLER_NUM;i++)
	{
		if(p_i2c_controllers[i] && (!strcmp(name,p_i2c_controllers[i]->name)))
		{
			p_i2c_con_selected = p_i2c_controllers[i];
			return 0;
		}
	}
	return -1;
}

/* 实现 i2c_transfer 接口函数 */

int i2c_transfer(p_i2c_msg msgs, int num)//num 是p_i2c_msg的个数也就是i2c_msg结构体的的个数
{
	return p_i2c_con_selected->master_xfer(msgs,num);
}
void i2c_init(void)
{
	/*注册下面的i2c控制器*/
	s3c2440_i2c_con_add();//将已经存在的机构体变量放到结构体指针数组中
	/*选择某款i2c控制器*/
	select_i2c_controller("s3c2440");//在已经存在的结构图数组中查找是否存在此名字的
	/*调用它的init函数*/
	p_i2c_con_selected->init();//执行p_i2c_controllers结构图初始化操作
}

