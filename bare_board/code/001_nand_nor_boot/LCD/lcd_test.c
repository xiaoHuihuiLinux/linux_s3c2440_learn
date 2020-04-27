#include "geometry.h"
void  lcd_test(void)
{
	unsigned int fb_base;
	int xres,yres,bpp;
	int x,y;
	unsigned short *p;
	unsigned int *p2;
	/*���ȳ�ʼ��*/
	lcd_init();//1.ע��lcd lcd������ 2.ѡ��ĳ��lcd lcd������3.ʹ��lcd���� ��ʼ��lcd������
	/*ʹ��*/
	lcd_enable();//ʹ��lcd������
	/*Ҫд֮ǰ���Ȼ��lcd���� fb_base,xres,yres,bpp*/
	get_lcd_params(&fb_base,&xres,&yres,&bpp);//���ִ��εķ�ʽ�Ƚ��ر�����˼���ں����и��βεĸ�ֵ
	fb_get_lcd_params();//Ҫ�õ�������������õ��Ļ�draw_line �е�fb_put_pixel �����û��Ч����
	/*��framebufferд����*/
	if(bpp == 16)
	{
		/*565 ��ʽ 5bit ��ɫ 0xf800*/
		p = (unsigned short*)fb_base;
		for(x = 0;x < xres; x++)
		{
			for(y = 0;y < yres; y++)
			{
				 *p++ =  0xf800;
			}
		}
		/*��*/
		p = (unsigned short*)fb_base;
		for(x = 0;x < xres; x++)
		
			for(y = 0;y < yres; y++)
			
				 *p++ =  0x7e0;
			
		/*��*/
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
		/*32 RRGGBB ��ʵ��24bit ���� �ǰ�24bit������һ�ֽ�*/ 
		p2 = (unsigned int*)fb_base;
		for(x = 0;x < xres; x++)
		{
			for(y = 0;y < yres; y++)
			{
				 *p2++ =  0xFF0000;
			}
		}
		/*��*/
		p2 = (unsigned int*)fb_base;
		for(x = 0;x < xres; x++)
		
			for(y = 0;y < yres; y++)
			
				 *p2++ =  0x00FF00;
			
		/*��*/
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
	/*����*/
	draw_line(0, 0, xres - 1, 0, 0xff0000);// ��ɫ�����ε����ϱ�
	draw_line(xres - 1, 0, xres - 1, yres - 1, 0xffff00);
	draw_line(0, yres - 1, xres - 1, yres - 1, 0xff00aa);//�������±�
	draw_line(0, 0, 0, yres - 1, 0xff00ef);
	draw_line(0, 0, xres - 1, yres - 1, 0xff4500);//�Խ��� \
	draw_line(xres - 1, 0, 0, yres - 1, 0xff0780);//�Խ��� /
	delay(1000000);
	/*��Բ*/
	draw_circle(xres/2, yres/2, yres/4, 0xff00);//ԭ�� ���� ��ɫ
}
