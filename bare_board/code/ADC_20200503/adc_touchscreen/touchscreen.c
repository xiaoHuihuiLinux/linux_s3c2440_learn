#include "../s3c2440_soc.h"

#define ADC_INT_BIT (10) //根据芯片手册INTSUBMSK
#define TC_INT_BIT (9)//根据芯片手册INTSUBMSK
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
void enter_wait_pen_down_mode(void)//触摸等待松开
{
	ADCTSC  = WAIT_PEN_DOWN |PULLUP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE  |XM_DISABLE |WAIT_INT_MODE;
	
}
void enter_wait_pen_up_mode(void)//
{
	ADCTSC  = WAIT_PEN_UP | PULLUP_ENABLE | YM_ENABLE | YP_DISABLE | XP_DISABLE  |XM_DISABLE |WAIT_INT_MODE;
}
/*测量触控点的位置*/
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
		enter_wait_pen_down_mode();//等待按下
	}
	if(ADCUPDN & (1 << 0))
	{
		printf("pen down\n\r");
		/*进入等待触摸笔松开模式*/
		enter_wait_pen_up_mode();//等待松开
	}
	#endif//通过打印可知ADCUPDN 低两位都是11所以不能判断
	if(ADCDAT0 & (1 << 15))//所以用ADCDAT0判断
	{
		//printf("pen up\n\r");
		enter_wait_pen_down_mode();//要做这一步检测动作等待按下
	}
	else 
	{
		//printf("pen down\n\r");
		#if 0
		/*进入等待触摸笔松开模式*/
		enter_wait_pen_up_mode();//等待松开
		#endif
		/*进入自动测量模式*/
		enter_auto_measure_mode();
		/*启动ADC*/
		ADCCON |= ( 1 << 0);
	}
}
void Isr_Adc(void)
{
	int x = ADCDAT0;
	int y = ADCDAT1 ;
	if( !(x & (1 << 15)) )/*仍然按下才打印否则可能打印错误值0表示按下*/
	{
		x &= 0x3FFF;
		y &= 0x3FFF;
		printf("x = %08d,y = %08d\n\r",x ,y);
	}
	enter_wait_pen_up_mode();//等待松开
}
#if 0
void Isr_Adc(void)//判断按下还是松开
{
}
#endif
void AdcTsIntHandle(int irq)//总的中断
{
	if(SUBSRCPND & (1 << TC_INT_BIT))//触摸屏中断
	{
		Isr_Tc();//中断处理函数
	}
	
	if(SUBSRCPND & (1 << ADC_INT_BIT) )//ADC中断
	{
		Isr_Adc();
	}
	SUBSRCPND=  (1 << TC_INT_BIT) | (1 << ADC_INT_BIT);//清除中断根据手册写入数据清掉
}
void adc_ts_int_init(void)
{
	/*注册中断处理函数*/
	register_irq(31,AdcTsIntHandle);//由数据手册可知是31号中断
	/*使能中断*/
	INTSUBMSK &= ~( (1 << ADC_INT_BIT) | (1 << TC_INT_BIT) );
	//INTMSK  &= ~(1 << INT_ADC_TS);

}
void adc_ts_req_init(void)
{
	/* [15] : ECFLG,  1 = End of A/D conversion
	 * [14] 预分频 : PRSCEN, 1 = A/D converter prescaler enable
	 * [13:6]:预分频值 PRSCVL, adc clk = PCLK / (PRSCVL + 1)
	 * [5:3] : SEL_MUX, 000 = AIN 0
	 * [2]   : STDBM
	 * [0]   : 1 = A/D conversion starts and this bit is cleared after the startup.
	 */
	ADCCON = (1 << 14) | (49 << 6) | (0<<3);
	/*按下触摸屏延迟一会产生中断
	延迟的时间 = ADCDLY *晶振周期 = ADCDLY* 1/12000000 =5ms 12mHz晶振*/
	ADCDLY = 60000;//延迟时间	
}


void touchscreen_init(void)
{	
	/*设置中断*/
	adc_ts_int_init();
	/*设置触摸屏接口：寄存器*/
	adc_ts_req_init();
	/*让触摸屏控制器进入等待中断模式*/
	enter_wait_pen_down_mode();
}
