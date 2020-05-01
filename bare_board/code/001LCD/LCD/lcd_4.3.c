#include "lcd.h"

#define LCD_FB_BASE  0x33c00000
lcd_params lcd_4_3_params = {
	.name = "lcd_4.3",
	.pins_pol = {
		.de    = NORMAL,	/* normal: �ߵ�ƽʱ���Դ������� */
		.pwren = NORMAL,    /* normal: �ߵ�ƽ��Ч */
		.vclk  = NORMAL,	/* normal: ���½��ػ�ȡ���� */
		.rgb   = NORMAL,	/* normal: �ߵ�ƽ��ʾ1 */
		.hsync = INVERT,    /* normal: ������ */
		.vsync = INVERT, 	/* normal: ������ */
	},
	.time_seq = {
		/* ��ֱ���� */
		.tvp=	10, /* vysnc������ */
		.tvb=	2,  /* �ϱߺڿ�, Vertical Back porch */
		.tvf=	2,  /* �±ߺڿ�, Vertical Front porch */

		/* ˮƽ���� */
		.thp=	41, /* hsync������ */
		.thb=	2,  /* ��ߺڿ�, Horizontal Back porch */
		.thf=	2,  /* �ұߺڿ�, Horizontal Front porch */

		.vclk=	9,  /* MHz */
	},
	/*Horizontal signal =525 = 41 + 2+ 2+ 480*/
	.xres = 480 ,//Horizontal display period
	.yres = 272,//Vertical display period
	.bpp = 32,//  16 or 32 not 24bit 
	.fb_base = LCD_FB_BASE,
};
void lcd_4_3_add(void)
{
	register_lcd(&lcd_4_3_params);
}
