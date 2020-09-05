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

static struct input_dev*buttons_dev; //input_dev����Ӳ���豸��Ϣ
static struct timer_list buttons_timer;//��ʱ��
static struct pin_desc *irq_pd;
struct pin_desc{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};
struct pin_desc pins_desc[4] = {
	{IRQ_EINT0, "S2",S3C2410_GPF0, KEY_L},//ע��S3C2410_GPF0 SC�ȵĴ�Сд
	{IRQ_EINT2, "S3",S3C2410_GPF2, KEY_S},
	{IRQ_EINT11,"S4",S3C2410_GPG3, KEY_ENTER},
	{IRQ_EINT19,"S5",S3C2410_GPG11,KEY_LEFTSHIFT},
};
static void buttons_timer_function(unsigned long data)//
{
	struct pin_desc*pindesc = irq_pd;//�ڰ���ע���жϵ�ʱ���������ǽ�irq_pd device_id����irq_pd�������
	/*��ǰ��һ���ֶ������жϴ���������ɵ�*/
	unsigned int pinval;
	if(!pindesc)//��Ϊ������seventh_drv_init����expires��ʱʱ����0���Բ��ܰ����Ƿ��¶���ִ�ж�ʱ��������������Ҫ�ж��Ƿ��а����¼�
		return;
	/*��ȡ����ֵ*/
	pinval = s3c2410_gpio_getpin(pindesc->pin);//��ֱ�Ӳ���dat�Ĵ�����һ����
	if(pinval)
	{
		/*�ɿ�*/	
		//���һ������ 0 �ɿ� 1 -����
		//�������豸input_dev�ġ�h_list�� �������ÿһ����Ա�������м������ӵ�ֻ��".dev"��".handler"
		//��Ա�ġ�input_handle���ṹ��������Щ��input_handle���ṹȡ��������handle��.
		//�������Ǵ�ʱ���͵���������塰input_handle���ṹ�����ĳ�Ա��.handler���ġ�event()��������
		input_event(buttons_dev,EV_KEY,pindesc->key_val,0);//�ϱ��¼�ע������������pindesc ������pins_desc
		input_sync(buttons_dev);//�ǵ�Ҫ�����ϱ�ͬ���¼�
	}
	else 
	{
		/*����*/	
	//ԭ�ͣ�void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)��
		input_event(buttons_dev,EV_KEY,pindesc->key_val,1);//�ϱ��¼�
		input_sync(buttons_dev);
	}
}
/*����������״̬*/
static irqreturn_t buttons_irq(int irq,void *dev_id)//�жϷ������Ĳ������жϺź�dev_id
{
	//10ms��������ʱ��
	//����jiffies 50 .HZ 100����ʱ��buttons_timer�ĳ�ʱʱ���� jiffies+HZ/100 =51  .ϵͳʱ����10ms����һ�����жϣ�Ȼ��jiffies��һ
	//jiffies ��Ϊ51 ϵͳʱ���жϴ�����ȥ�ҵ���Ӧ���Ѿ�����ʱ��Ķ�ʱ��
	irq_pd  = (struct pin_desc*)dev_id;
	mod_timer(&buttons_timer,jiffies+HZ/100);//jiffies ��ȫ�ֱ��� 
	return IRQ_RETVAL(IRQ_HANDLED);//��һ�β�֪��ΪʲĪż���Ĵ�ӡ���������Ƿ���return IRQ_HANDLED; ����
}
static int buttons_init(void)
{
	int i;
	/*1 ������һ��input_dev�ṹ��*/
	buttons_dev = input_allocate_device();//����һ��input_dev ���������Ҫ�жϴ�Input_dev���ṹ�Ƿ����ɹ���������Ϊ�򻯴��벻���жϡ�
	/*2������*/
	/*2.1�ܲ��������¼�*/
	/*���Ǽ�����Ҫ���ǵ��ĸ���������L S ENTER SHIFT  ,��ǰд������ʱ��ֻ������Ӧ�ó���֪����ʲô��˼��������Ҫдһ��ͨ�õĳ��� ����
	���Ƕ����L S enter ��shift*/
	set_bit(EV_KEY,buttons_dev->evbit);//����evbit��ֵ��ĳһλλEV_KEYb��ʾ�ܲ����������¼�
	set_bit(EV_REP,buttons_dev->evbit);//�ظ����¼��������ǰ���֮��һֱ�ϱ��¼�
	/*2.2�ܲ��������¼�����Щ�¼� L ,S, enter, LEFTSHIT*/
	set_bit(KEY_L,buttons_dev->keybit);//L��
	set_bit(KEY_S,buttons_dev->keybit);//S��
	set_bit(KEY_ENTER,buttons_dev->keybit);//�س���
	set_bit(KEY_LEFTSHIFT,buttons_dev->keybit);//��shirt ��
	/*3��ע��*/
	/*�������input_handler_list�е�ÿһ��handler Ȼ�� input_attach_handler
	��dev�봦��ʽ�е�id_table�Ƚ��Ƿ���ƥ����ƥ��Ļ�����connect������
	��Բ�ͬ��connect����������һ��input_handle�ṹ��Ȼ��input_register_handle����
	��handle���������ұߵ�h_list������*/
	input_register_device(buttons_dev);//��ʵҲ���ǽ������input_dev�ṹ����뵽input_dev������
	/*4��Ӳ����صĲ���*/
	//��ʱ��
	init_timer(&buttons_timer);//��ʱ����ʼ��
	buttons_timer.function = buttons_timer_function;//data�Ǹ�function���ݵĲ���
	add_timer(&buttons_timer);//�����ں�jiffies >0
	for(i = 0; i < 4;i++)
	{
		//ע���ж�
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
module_init(buttons_init);//�������������ǲ��ܱ�����
module_exit(buttons_exit);

MODULE_LICENSE("GPL");

