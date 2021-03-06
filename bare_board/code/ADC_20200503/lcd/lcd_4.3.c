
#include "lcd.h"

#define LCD_FB_BASE 0x33c00000

lcd_params lcd_4_3_params = {
	.name = "lcd_4.3",
	.pins_pol = {
		.de    = NORMAL,	/* normal: 高电平时可以传输数据 */
		.pwren = NORMAL,    /* normal: 高电平有效 */
		.vclk  = NORMAL,	/* normal: 在下降沿获取数据 */
		.rgb   = NORMAL,	/* normal: 高电平表示1 */
		.hsync = INVERT,    /* normal: 高脉冲 */
		.vsync = INVERT, 	/* normal: 楂樿剦鍐� */
	},
	.time_seq = {
		/* 鍨傜洿鏂瑰悜*/
		.tvp=	10, /* vysnc鑴夊啿瀹藉害 */
		.tvb=	2,  /* 涓婇粦妗�, Vertical Back porch */
		.tvf=	2,  /* 涓嬭竟榛戞, Vertical Front porch */

		/* 姘村钩鏂瑰悜 */
		.thp=	41, /* hsync鑴夊啿瀹藉害 */
		.thb=	2,  /* 鍙宠竟榛戞, Horizontal Back porch */
		.thf=	2,  /* 宸﹁竟榛戞, Horizontal Front porch */

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

