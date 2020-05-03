
#include "lcd.h"

#define LCD_FB_BASE 0x33c00000

lcd_params lcd_4_3_params = {
	.name = "lcd_4.3",
	.pins_pol = {
		.de    = NORMAL,	/* normal: �ߵ�ƽʱ���Դ������� */
		.pwren = NORMAL,    /* normal: �ߵ�ƽ��Ч */
		.vclk  = NORMAL,	/* normal: ���½��ػ�ȡ���� */
		.rgb   = NORMAL,	/* normal: �ߵ�ƽ��ʾ1 */
		.hsync = INVERT,    /* normal: ������ */
		.vsync = INVERT, 	/* normal: 高脉冲 */
	},
	.time_seq = {
		/* 垂直方向*/
		.tvp=	10, /* vysnc脉冲宽度 */
		.tvb=	2,  /* 上黑框, Vertical Back porch */
		.tvf=	2,  /* 下边黑框, Vertical Front porch */

		/* 水平方向 */
		.thp=	41, /* hsync脉冲宽度 */
		.thb=	2,  /* 右边黑框, Horizontal Back porch */
		.thf=	2,  /* 左边黑框, Horizontal Front porch */

		.vclk=	9,  /* MHz */
	},
	.xres = 480,
	.yres = 272,
	.bpp  = 32,  /* 16, no 24bpp */
	.fb_base = LCD_FB_BASE,
};


void lcd_4_3_add(void)
{
	register_lcd(&lcd_4_3_params);
}

