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
static volatile struct s3c_ts_regs *s3c_ts_regs;//����volatile
void start_adc(void)
{
	s3c_ts_regs->adcon |= (1 << 0);//ADCCON ADC ���ƼĴ���bit0 ����1 ��������
}
void enter_measure_xy_mode(void)
{
	s3c_ts_regs->adctsc = (1<<3) | (1<<2); //ACTSC ��bit3 ����Ϊ1(��������ʹ�ܸ������ǵ��ֲ�NOTES��֪).bit2 ����Ϊ1.
}

static void enter_wait_pen_down_mode(void)//����ȴ������ʰ���ģʽ��
{
	s3c_ts_regs->adctsc = 0xd3;//bit8����Ϊ0��� ����down �ж�
}
static void enter_wait_pen_up_mode(void)//����Ҫ�ֶ����������������up ���� down�ж�
{
	//���˼Ĵ����� adctsc �Ĵ�������Ϊ 0xd3 �ͽ��롰�ȴ��ж�ģʽ����
	s3c_ts_regs->adctsc = 0x1d3; //ACTSC ��bit8 ����Ϊ1��� ����up �ж�
}
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)//�жϴ������а��º��ɿ�
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
		//1.����ΪAuto x yģʽ
		enter_measure_xy_mode();
		//2.stat adc ת��
		start_adc();//ADC ������������˲����ɣ�������������Ӧ���и��жϲ����ĵ�XY �����ѹֵ
	}
	return IRQ_HANDLED;
}
//ʵ�� ADC ������ɺ������жϷ�����򣬴�XY ����ֵ(��ѹֵ)��
static irqreturn_t adc_irq(int irq,void *dev_id)
{
	static int cnt =0;
	//ADCDAT0 �Ĵ�ĵ�10 λ����X ����ֵ.ADCDAT1 ��10 ����Y ����ֵ.
	printk("adc_irq_cnt = %d, x= %d,y=%d\n",++cnt,s3c_ts_regs->adcdat0 &
	0x3ff, s3c_ts_regs->adcdat1 & 0x3ff);//0x3FFF����10λadc�����ֵ
	/*
	���ӵĻ����ǵ㶯ʱû���жϷ�������������������Ӧ��������������ν�ġ��ȴ��ɿ�ģʽ--enter_wait_pen_up_mode()��������
	�����Ӳ���
	*/
	enter_wait_pen_up_mode();
	return IRQ_HANDLED;
}

static int  s3c_ts_init(void)
{
	/*����inpu_dev�ṹ��*/
	s3c_ts_dev = input_allocate_device();
	/*����*/
	//һ�����û����ʲô���͵��¼���Ȼ�������¼���Ӧʲô�¼�
	set_bit(EV_KEY, s3c_ts_dev->evbit); //�������¼�.
	set_bit(EV_ABS, s3c_ts_dev->evbit); //�������Ǿ���λ���¼�(��������λ��).
	//1.2.2,�ܲ��������¼��е���Щ�¼���
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit); //����������������Ĵ������¼���
	/* input_set_abs_params(struct input_dev *dev, int axis, int min, int max, int fuzz, int flat)*/
	//����1input_dev�ṹ�壬2.axis x��������� 3.��Сֵ4.���ֵ ���Ĳ���������
	/*
	�ֲ���˵������������Բ�����10 λ����������ʵ������һ�� ADC ת������10 λ��
	��0x3FF.0x3FF �Ķ������ǡ�1 111 111 111��һ��10 λ��
	*/
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);//x ����
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);//y ����
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 0x3FF, 0, 0);//ѹ�����򼶱�ֻ��0 ��1��ͼ����ѹ��Խ�󣬻�����������Խ�֣��ʼ�Խ�֣���
	/*ע��*/
	input_register_device(s3c_ts_dev);
	/*Ӳ����صĲ���*/
	//1.ʹ��ʱ�Ӹ����Ѿ����ڵ�s3c2410_ts.c�Ϸ���
	clk = clk_get(NULL, "adc");//ʹ��ʱ��
	clk_enable(clk);
	//2��ַӳ��
	s3c_ts_regs = ioremap(0x58000000,sizeof(s3c_ts_regs));
	//����S3C2440��ADC/TS�Ĵ���������
	/* bit[14]  : 1-A/D converter prescaler enable
	 * bit[13:6]: A/D converter prescaler value,
	 *            49, ADCCLK=PCLK/(49+1)=50MHz/(49+1)=1MHz
	 * bit[0]: A/D conversion starts by enable. ����Ϊ0
	 */
	 s3c_ts_regs->adcon = (1<<14)|(49<<6);
	//ע�� INT_TC��touch change�� �жϣ�
	//int request_irq(unsigned int irq, irq_handler_t handler,unsigned long irqflags, const char *devname, void *dev_id){}
	request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	//adcת����ɻ����һ���ж���������Ҫע���ж�
	request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);
	//ע�������ʵ�ְ��²����ж��ɿ��ֲ����жϣ�Ҳ���ǵȴ��ж�ģʽ
	enter_wait_pen_down_mode();//�ȴ�����
	return 0;
}

static void  s3c_ts_exit(void)
{
	//�ͷ�IRQ_TC
	free_irq(IRQ_TC,NULL);//���� �жϵ�����
	//�ͷ�IRQ_ADC �ж�
	free_irq(IRQ_ADC, NULL);
	iounmap(s3c_ts_regs);
	//���ں��ͷŴ� s3c_ts_dev �ṹ��
	input_unregister_device(s3c_ts_dev);
	//�ͷ� s3c_ts_dev �ṹ�ռ�.
	input_free_device(s3c_ts_dev);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");

