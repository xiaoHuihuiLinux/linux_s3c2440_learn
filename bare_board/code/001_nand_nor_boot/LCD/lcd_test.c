#include "geometry.h"
void  lcd_test(void)
{
	unsigned int fb_base;
	int xres,yres,bpp;
	int x,y;
	unsigned short *p;
	unsigned int *p2;
	/*首先初始化*/
	lcd_init();//1.注册lcd lcd控制器 2.选择某款lcd lcd控制器3.使用lcd参数 初始化lcd控制器
	/*使能*/
	lcd_enable();//使能lcd控制器
	/*要写之前首先获的lcd参数 fb_base,xres,yres,bpp*/
	get_lcd_params(&fb_base,&xres,&yres,&bpp);//这种传参的方式比较特别有意思，在函数中给形参的赋值
	fb_get_lcd_params();//要得到这个参数，不得到的话draw_line 中的fb_put_pixel 这个就没有效果了
	/*向framebuffer写数据*/
	if(bpp == 16)
	{
		/*565 格式 5bit 红色 0xf800*/
		p = (unsigned short*)fb_base;
		for(x = 0;x < xres; x++)
		{
			for(y = 0;y < yres; y++)
			{
				 *p++ =  0xf800;
			}
		}
		/*绿*/
		p = (unsigned short*)fb_base;
		for(x = 0;x < xres; x++)
		
			for(y = 0;y < yres; y++)
			
				 *p++ =  0x7e0;
			
		/*蓝*/
		p = (unsigned short*)fb_base;
		for(x = 0;x < xres; x++)
		
			for(y = 0;y < yres; y++)
			
				 *p++ =  0x1f;
		/* black */
		p = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p++ = 0;
			
	}
	else if(bpp == 32)
	{
		/*32 RRGGBB 其实是24bit 但是 是按24bit填充空了一字节*/ 
		p2 = (unsigned int*)fb_base;
		for(x = 0;x < xres; x++)
		{
			for(y = 0;y < yres; y++)
			{
				 *p2++ =  0xFF0000;
			}
		}
		/*绿*/
		p2 = (unsigned int*)fb_base;
		for(x = 0;x < xres; x++)
		
			for(y = 0;y < yres; y++)
			
				 *p2++ =  0x00FF00;
			
		/*蓝*/
		p2 = (unsigned int*)fb_base;
		for(x = 0;x < xres; x++)
		
			for(y = 0;y < yres; y++)
			
				 *p2++ =  0x0000FF;
		/* black */
		p = (unsigned short *)fb_base;
		for (x = 0; x < xres; x++)
			for (y = 0; y < yres; y++)
				*p++ = 0;
			
	}
	delay(100000);
	/*画线*/
	draw_line(0, 0, xres - 1, 0, 0xff0000);// 颜色正方形的最上边
	draw_line(xres - 1, 0, xres - 1, yres - 1, 0xffff00);
	draw_line(0, yres - 1, xres - 1, yres - 1, 0xff00aa);//正方形下边
	draw_line(0, 0, 0, yres - 1, 0xff00ef);
	draw_line(0, 0, xres - 1, yres - 1, 0xff4500);//对角线 \
	draw_line(xres - 1, 0, 0, yres - 1, 0xff0780);//对角线 /
	delay(1000000);
	/*画圆*/
	draw_circle(xres/2, yres/2, yres/4, 0xff00);//原点 坐标 颜色
}
