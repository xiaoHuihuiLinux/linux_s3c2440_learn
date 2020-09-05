#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>


static struct input_dev *s3c_ts_dev;
struct clk	*clk;

struct s3c_ts_regs
{
	unsigned long adcon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};
static volatile struct s3c_ts_regs *s3c_ts_regs;//加上volatile
void start_adc(void)
{
	s3c_ts_regs->adcon |= (1 << 0);//ADCCON ADC 控制寄存器bit0 设置1 即可启动
}
void enter_measure_xy_mode(void)
{
	s3c_ts_regs->adctsc = (1<<3) | (1<<2); //ACTSC 的bit3 设置为1(禁用上拉使能根据我们的手册NOTES得知).bit2 设置为1.
}

static void enter_wait_pen_down_mode(void)//进入等待触摸笔按下模式。
{
	s3c_ts_regs->adctsc = 0xd3;//bit8设置为0检测 就是down 中断
}
static void enter_wait_pen_up_mode(void)//我们要手动的设置它让他检测up 或者 down中断
{
	//将此寄存器的 adctsc 寄存器设置为 0xd3 就进入“等待中断模式”。
	s3c_ts_regs->adctsc = 0x1d3; //ACTSC 的bit8 设置为1检测 就是up 中断
}
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)//判断处理函数有按下和松开
{
	if (s3c_ts_regs->adcdat0 & (1<<15))//ADCDAT0  bit15   0 down / 1 up
	{
		printk("pen up\n");
		enter_wait_pen_down_mode();
	}
	else
	{
		//printk("pen down\n");
		//enter_wait_pen_up_mode();
		//1.设置为Auto x y模式
		enter_measure_xy_mode();
		//2.stat adc 转换
		start_adc();//ADC 的启动不可能瞬间完成（不会死等它）应该有个中断产生的到XY 坐标电压值
	}
	return IRQ_HANDLED;
}
//实现 ADC 启动完成后进入的中断服务程序，打开XY 坐标值(电压值)。
static irqreturn_t adc_irq(int irq,void *dev_id)
{
	static int cnt =0;
	//ADCDAT0 寄存的低10 位就是X 坐标值.ADCDAT1 低10 就是Y 坐标值.
	printk("adc_irq_cnt = %d, x= %d,y=%d\n",++cnt,s3c_ts_regs->adcdat0 &
	0x3ff, s3c_ts_regs->adcdat1 & 0x3ff);//0x3FFF就是10位adc的最大值
	/*
	不加的话我们点动时没有中断服务程序来处理。测量完后，应该再让它进入所谓的“等待松开模式--enter_wait_pen_up_mode()”这样才
	能连接操作
	*/
	enter_wait_pen_up_mode();
	return IRQ_HANDLED;
}

static int  s3c_ts_init(void)
{
	/*分配inpu_dev结构体*/
	s3c_ts_dev = input_allocate_device();
	/*设置*/
	//一般设置会产生什么类型的事件，然后这类事件对应什么事件
	set_bit(EV_KEY, s3c_ts_dev->evbit); //按键类事件.
	set_bit(EV_ABS, s3c_ts_dev->evbit); //触摸屏是绝对位移事件(鼠标是相对位移).
	//1.2.2,能产生这类事件中的哪些事件。
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit); //产生“按键”类里的触摸屏事件。
	/* input_set_abs_params(struct input_dev *dev, int axis, int min, int max, int fuzz, int flat)*/
	//参数1input_dev结构体，2.axis x方向的坐标 3.最小值4.最大值 最后的参数不关心
	/*
	手册上说这个触摸屏可以产生“10 位”。触摸屏实质上是一个 ADC 转换器。10 位就
	是0x3FF.0x3FF 的二进制是“1 111 111 111”一共10 位。
	*/
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);//x 方向
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);//y 方向
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 0x3FF, 0, 0);//压力方向级别只有0 与1画图板是压力越大，画出来的线条越粗（笔迹越粗）。
	/*注册*/
	input_register_device(s3c_ts_dev);
	/*硬件相关的操作*/
	//1.使能时钟根据已经存在的s3c2410_ts.c上分析
	clk = clk_get(NULL, "adc");//使能时钟
	clk_enable(clk);
	//2地址映射
	s3c_ts_regs = ioremap(0x58000000,sizeof(s3c_ts_regs));
	//设置S3C2440的ADC/TS寄存器的设置
	/* bit[14]  : 1-A/D converter prescaler enable
	 * bit[13:6]: A/D converter prescaler value,
	 *            49, ADCCLK=PCLK/(49+1)=50MHz/(49+1)=1MHz
	 * bit[0]: A/D conversion starts by enable. 先设为0
	 */
	 s3c_ts_regs->adcon = (1<<14)|(49<<6);
	//注册 INT_TC（touch change） 中断：
	//int request_irq(unsigned int irq, irq_handler_t handler,unsigned long irqflags, const char *devname, void *dev_id){}
	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	//adc转换完成会产生一个中断所以我们要注册中断
	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);
	//注册完如何实现按下产生中断松开又产生中断，也就是等待中断模式
	enter_wait_pen_down_mode();//等待按下
	return 0;
}

static void  s3c_ts_exit(void)
{
	//释放IRQ_TC
	free_irq(IRQ_TC,NULL);//参数 中断的类型
	//释放IRQ_ADC 中断
	free_irq(IRQ_ADC, NULL);
	iounmap(s3c_ts_regs);
	//从内核释放此 s3c_ts_dev 结构体
	input_unregister_device(s3c_ts_dev);
	//释放 s3c_ts_dev 结构空间.
	input_free_device(s3c_ts_dev);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");

