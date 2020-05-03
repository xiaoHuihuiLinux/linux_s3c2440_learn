
#include "lcd.h"

#define LCD_FB_BASE 0x33c00000

lcd_params lcd_4_3_params = {
	.name = "lcd_4.3",
	.pins_pol = {
		.de    = NORMAL,	/* normal: ¸ßµçÆ½Ê±¿ÉÒÔ´«ÊäÊı¾İ */
		.pwren = NORMAL,    /* normal: ¸ßµçÆ½ÓĞĞ§ */
		.vclk  = NORMAL,	/* normal: ÔÚÏÂ½µÑØ»ñÈ¡Êı¾İ */
		.rgb   = NORMAL,	/* normal: ¸ßµçÆ½±íÊ¾1 */
		.hsync = INVERT,    /* normal: ¸ßÂö³å */
		.vsync = INVERT, 	/* normal: é«˜è„‰å†² */
	},
	.time_seq = {
		/* å‚ç›´æ–¹å‘*/
		.tvp=	10, /* vysncè„‰å†²å®½åº¦ */
		.tvb=	2,  /* ä¸Šé»‘æ¡†, Vertical Back porch */
		.tvf=	2,  /* ä¸‹è¾¹é»‘æ¡†, Vertical Front porch */

		/* æ°´å¹³æ–¹å‘ */
		.thp=	41, /* hsyncè„‰å†²å®½åº¦ */
		.thb=	2,  /* å³è¾¹é»‘æ¡†, Horizontal Back porch */
		.thf=	2,  /* å·¦è¾¹é»‘æ¡†, Horizontal Front porch */

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

