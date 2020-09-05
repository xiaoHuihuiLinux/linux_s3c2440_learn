#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>

#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/regs-gpio.h>

static struct input_dev*buttons_dev; //input_dev描述硬件设备信息
static struct timer_list buttons_timer;//定时器
static struct pin_desc *irq_pd;
struct pin_desc{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};
struct pin_desc pins_desc[4] = {
	{IRQ_EINT0, "S2",S3C2410_GPF0, KEY_L},//注意S3C2410_GPF0 SC等的大小写
	{IRQ_EINT2, "S3",S3C2410_GPF2, KEY_S},
	{IRQ_EINT11,"S4",S3C2410_GPG3, KEY_ENTER},
	{IRQ_EINT19,"S5",S3C2410_GPG11,KEY_LEFTSHIFT},
};
static void buttons_timer_function(unsigned long data)//
{
	struct pin_desc*pindesc = irq_pd;//在按键注册中断的时候我们我们将irq_pd device_id给了irq_pd这个变量
	/*以前这一部分都是在中断处理函数中完成的*/
	unsigned int pinval;
	if(!pindesc)//因为我们在seventh_drv_init设置expires超时时间是0所以不管按键是否按下都会执行定时器处理函数，所以要判断是否有按键事件
		return;
	/*读取引脚值*/
	pinval = s3c2410_gpio_getpin(pindesc->pin);//跟直接操作dat寄存器是一样的
	if(pinval)
	{
		/*松开*/	
		//最后一个参数 0 松开 1 -按下
		//对输入设备input_dev的“h_list” 链表里的每一个成员（就是中间作连接的只有".dev"和".handler"
		//成员的“input_handle”结构）。把这些“input_handle”结构取出来到“handle”.
		//若它们是打开时，就调用这个具体“input_handle”结构变量的成员“.handler”的“event()”函数。
		input_event(buttons_dev,EV_KEY,pindesc->key_val,0);//上报事件注意我们这里用pindesc 而不是pins_desc
		input_sync(buttons_dev);//记得要加上上报同步事件
	}
	else 
	{
		/*按下*/	
	//原型：void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)。
		input_event(buttons_dev,EV_KEY,pindesc->key_val,1);//上报事件
		input_sync(buttons_dev);
	}
}
/*读出按键的状态*/
static irqreturn_t buttons_irq(int irq,void *dev_id)//中断服务程序的参数是中断号和dev_id
{
	//10ms后启动定时器
	//假设jiffies 50 .HZ 100。定时器buttons_timer的超时时间是 jiffies+HZ/100 =51  .系统时钟是10ms产生一个中中断，然后jiffies加一
	//jiffies 就为51 系统时钟中断处理函数去找到相应的已经到达时间的定时器
	irq_pd  = (struct pin_desc*)dev_id;
	mod_timer(&buttons_timer,jiffies+HZ/100);//jiffies 是全局变量 
	return IRQ_RETVAL(IRQ_HANDLED);//第一次不知道为什莫偶发的打印按键按下是否是return IRQ_HANDLED; 导致
}
static int buttons_init(void)
{
	int i;
	/*1 ：分配一个input_dev结构体*/
	buttons_dev = input_allocate_device();//分配一个input_dev 正常情况下要判断此Input_dev”结构是否分配成功，但这里为简化代码不予判断。
	/*2：设置*/
	/*2.1能产生哪类事件*/
	/*我们假如想要我们的四个按键产生L S ENTER SHIFT  ,以前写驱动的时后只有我们应用程序知道是什么意思。那我们要写一个通用的程序 ‘、
	我们定义成L S enter 左shift*/
	set_bit(EV_KEY,buttons_dev->evbit);//设置evbit数值的某一位位EV_KEYb表示能产生按键类事件
	set_bit(EV_REP,buttons_dev->evbit);//重复类事件就是我们按下之后一直上报事件
	/*2.2能产生这类事件的哪些事件 L ,S, enter, LEFTSHIT*/
	set_bit(KEY_L,buttons_dev->keybit);//L键
	set_bit(KEY_S,buttons_dev->keybit);//S键
	set_bit(KEY_ENTER,buttons_dev->keybit);//回车键
	set_bit(KEY_LEFTSHIFT,buttons_dev->keybit);//左shirt 键
	/*3：注册*/
	/*他会遍历input_handler_list中的每一个handler 然后 input_attach_handler
	看dev与处理方式中的id_table比较是否有匹配有匹配的话调用connect函数，
	针对不同的connect函数。分配一个input_handle结构体然后input_register_handle（）
	把handle放入左右右边的h_list链表里*/
	input_register_device(buttons_dev);//其实也就是将分配的input_dev结构体加入到input_dev链表中
	/*4：硬件相关的操作*/
	//定时器
	init_timer(&buttons_timer);//定时器初始化
	buttons_timer.function = buttons_timer_function;//data是给function传递的参数
	add_timer(&buttons_timer);//告诉内核jiffies >0
	for(i = 0; i < 4;i++)
	{
		//注册中断
		request_irq(pins_desc[i].irq,buttons_irq,IRQT_BOTHEDGE,pins_desc[i].name,&pins_desc[i]);
	}
	return 0;
}
static void buttons_exit(void)
{
	int i;
	for(i = 0;i < 4;i++)
	{
		free_irq(pins_desc[i].irq,&pins_desc[i]);	
	}
	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);
}
module_init(buttons_init);//驱动中修饰他们才能被调用
module_exit(buttons_exit);

MODULE_LICENSE("GPL");

