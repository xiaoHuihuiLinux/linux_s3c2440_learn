void  lcd_test(void)
{
	unsigned int fb_base;
	int xres,yres,bpp;
	int x,y;
	unsigned short *p;
	unsigned int *p2;
	/*���ȳ�ʼ��*/
	lcd_init();
	/*ʹ��*/
	lcd_enable();
	/*Ҫд֮ǰ���Ȼ��lcd���� fb_base,xres,yres,bpp*/
	get_lcd_params(&fb_base,&xres,&yres,&bpp);
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
			
	}
}
