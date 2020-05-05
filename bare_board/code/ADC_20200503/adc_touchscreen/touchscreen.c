#include "../s3c2440_soc.h"

#define ADC_INT_BIT (10) //����оƬ�ֲ�INTSUBMSK
#define TC_INT_BIT (9)//����оƬ�ֲ�INTSUBMSK
#define INT_ADC_TS (31)
/*ADCTSC bits*/
#define UP_DOWN_STAT_BIT (8)
/*Detect Stylus Up or Down status.
0 = Detect Stylus Down Interrupt Signal.
1 = Detect Stylus Up Interrupt Signal.*/
#define WAIT_PEN_DOWN  (0 << UP_DOWN_STAT_BIT)//Detect Stylus Up or Down status
#define WAIT_PEN_UP	   (1 << UP_DOWN_STAT_BIT)

#define YM_ENABLE (1 << 7)
#define YM_DISABLE (0 << 7)
#define YP_ENABLE (0 << 6)
#define YP_DISABLE (1 << 6)

#define XM_ENABLE (1 << 5)
#define XM_DISABLE (0 << 5)
#define XP_ENABLE (0 << 4)
#define XP_DISABLE (1 << 4)

#define PULLUP_ENABLE (0 << 3)
#define PULLUP_DISABLE (1 << 3)

#define AUTO_PST (1 << 2)

#define WAIT_INT_MODE (3)
#define NO_OPR_MODE (0)
void enter_wait_pen_down_mode(void)//�����ȴ��ɿ�
{
	ADCTSC  = WAIT_PEN_DOWN |PULLUP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE  |XM_DISABLE |WAIT_INT_MODE;
	
}
void enter_wait_pen_up_mode(void)//
{
	ADCTSC  = WAIT_PEN_UP | PULLUP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE  |XM_DISABLE |WAIT_INT_MODE;
}
/*�������ص��λ��*/
void enter_auto_measure_mode(void)
{
	ADCTSC = AUTO_PST |  NO_OPR_MODE;
}
void Isr_Tc(void)
{
	//printf("ADCUPDN: 0x%x, ADCDAT0: 0x%x,ADCDAT1: 0x%x",ADCUPDN,ADCDAT0,ADCDAT1);
	#if 0
	if(ADCUPDN & (1 << 1))
	{
		printf("pen up\n\r");
		enter_wait_pen_down_mode();//�ȴ�����
	}
	if(ADCUPDN & (1 << 0))
	{
		printf("pen down\n\r");
		/*����ȴ��������ɿ�ģʽ*/
		enter_wait_pen_up_mode();//�ȴ��ɿ�
	}
	#endif//ͨ����ӡ��֪ADCUPDN ����λ����11���Բ����ж�
	if(ADCDAT0 & (1 << 15))//������ADCDAT0�ж�
	{
		//printf("pen up\n\r");
		enter_wait_pen_down_mode();//Ҫ����һ����⶯���ȴ�����
	}
	else 
	{
		//printf("pen down\n\r");
		#if 0
		/*����ȴ��������ɿ�ģʽ*/
		enter_wait_pen_up_mode();//�ȴ��ɿ�
		#endif
		/*�����Զ�����ģʽ*/
		enter_auto_measure_mode();
		/*����ADC*/
		ADCCON |= ( 1 << 0);
	}
}
void Isr_Adc(void)
{
	int x = ADCDAT0;
	int y = ADCDAT1 ;
	if( !(x & (1 << 15)) )/*��Ȼ���²Ŵ�ӡ������ܴ�ӡ����ֵ0��ʾ����*/
	{
		x &= 0x3FFF;
		y &= 0x3FFF;
		printf("x = %08d,y = %08d\n\r",x ,y);
	}
	enter_wait_pen_up_mode();//�ȴ��ɿ�
}
#if 0
void Isr_Adc(void)//�жϰ��»����ɿ�
{
}
#endif
void AdcTsIntHandle(int irq)//�ܵ��ж�
{
	if(SUBSRCPND & (1 << TC_INT_BIT))//�������ж�
	{
		Isr_Tc();//�жϴ�����
	}
	
	if(SUBSRCPND & (1 << ADC_INT_BIT) )//ADC�ж�
	{
		Isr_Adc();
	}
	SUBSRCPND=  (1 << TC_INT_BIT) | (1 << ADC_INT_BIT);//����жϸ����ֲ�д���������
}
void adc_ts_int_init(void)
{
	/*ע���жϴ�����*/
	register_irq(31,AdcTsIntHandle);//�������ֲ��֪��31���ж�
	/*ʹ���ж�*/
	INTSUBMSK &= ~( (1 << ADC_INT_BIT) | (1 << TC_INT_BIT) );
	//INTMSK  &= ~(1 << INT_ADC_TS);

}
void adc_ts_req_init(void)
{
	/* [15] : ECFLG,  1 = End of A/D conversion
	 * [14] Ԥ��Ƶ : PRSCEN, 1 = A/D converter prescaler enable
	 * [13:6]:Ԥ��Ƶֵ PRSCVL, adc clk = PCLK / (PRSCVL + 1)
	 * [5:3] : SEL_MUX, 000 = AIN 0
	 * [2]   : STDBM
	 * [0]   : 1 = A/D conversion starts and this bit is cleared after the startup.
	 */
	ADCCON = (1 << 14) | (49 << 6) | (0<<3);
	/*���´������ӳ�һ������ж�
	�ӳٵ�ʱ�� = ADCDLY *�������� = ADCDLY* 1/12000000 =5ms 12mHz����*/
	ADCDLY = 60000;//�ӳ�ʱ��	
}


void touchscreen_init(void)
{	
	/*�����ж�*/
	adc_ts_int_init();
	/*���ô������ӿڣ��Ĵ���*/
	adc_ts_req_init();
	/*�ô���������������ȴ��ж�ģʽ*/
	enter_wait_pen_down_mode();
}
