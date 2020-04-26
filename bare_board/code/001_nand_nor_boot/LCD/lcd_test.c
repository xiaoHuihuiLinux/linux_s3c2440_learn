void  lcd_test(void)
{
	unsigned int fb_base;
	int xres,yres,bpp;
	int x,y;
	unsigned short *p;
	unsigned int *p2;
	/*首先初始化*/
	lcd_init();
	/*使能*/
	lcd_enable();
	/*要写之前首先获的lcd参数 fb_base,xres,yres,bpp*/
	get_lcd_params(&fb_base,&xres,&yres,&bpp);
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
			
	}
}
