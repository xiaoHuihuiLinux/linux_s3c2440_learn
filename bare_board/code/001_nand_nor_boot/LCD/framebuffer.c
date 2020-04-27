/*实现画点*/
/*获得lcd参数,以防参数错误*/
#include "lcd.h"
static unsigned int fb_base;
static int xres,yres,bpp;
void fb_get_lcd_params(void)
{
	get_lcd_params(&fb_base,&xres,&yres,&bpp);//这种传参的方式比较特别有意思，在函数中给形参的赋值
}
/*
color :32bit
*/
unsigned short convert32bppto16bpp(unsigned int rgb)
{
	int r = (rgb >> 16)& 0xFF;
	int g = (rgb >> 8) & 0xFF;
	int b = rgb & 0xFF;
	/*rgb 565*/
	r = r >> 3;
	g = g >> 2;
	b = b >> 3;
	return ((r << 11) | (g << 5) | b);
}
void fb_put_pixel(int x,int y,unsigned int color)
{
	unsigned char * pc;/*8 bpp*/
	unsigned short *pw;/*16bpp*/
	unsigned int *pdw;/*32 bbpp*/
	unsigned int pixel_base = fb_base + (xres*bpp/8)*y + (x * bpp/8);
	switch(bpp)
	{
		case 8:
			pc = (unsigned char*)pixel_base;
			*pc = color; 
			break;
		case 16:
			pw = (unsigned short*)pixel_base;
			*pw= convert32bppto16bpp(color); 
			break;
		case 32:
			pdw = (unsigned int*)pixel_base;
			*pdw= (color);
			break;
		default:
			break;
	}
}


