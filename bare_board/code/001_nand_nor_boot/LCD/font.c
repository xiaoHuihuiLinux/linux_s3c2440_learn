
static unsigned int fb_base;
static int xres,yres,bpp;
extern const unsigned char fontdata_8x16[];

void font_init(void)//初始化
{
	get_lcd_params(&fb_base,&xres,&yres,&bpp);//这种传参的方式比较特别有意思，在函数中给形参的赋值
}

/*根据字母的点阵在lcd上画文字*/

void fb_print_char(int x,int y,char c,unsigned int color)
{	int i,j;	
	/*根据c的assic码在fondata——8x16得到点阵*/
	unsigned char * dots = &fontdata_8x16[c * 16]; //假如是A对应的ASSIC十进制65对应数组的第65个元素
	unsigned char data;
	int bit;
	/*根据点阵渲染对应像素颜色*/
	for(j = y;j < y + 16;j++)//字符的高
	{	data = *dots++;
		bit = 7;
		for(i = x;i < x +8;i++)//字符的宽
		{	
			/*根据点阵的某位是否描色*/
			if(data & (1 << bit) )
			{
				fb_put_pixel(i, j, color);
			}
			bit--;
		}
	}
}
/*abc\n\r123*/
void fb_print_string(int x,int y,char* str,unsigned int color)
{
	int i  =0,j;
	while(str[i])
	{
		if(str[i] == '\n')
		{
			y = y+16;//遇到换行
		}
		else if(str[i] == '\r')
		{
			x = 0;//行首
		}
		else
		{
			fb_print_char(x,y,str[i],color);
			x = x+8;
			if(x >= xres)/*超出怕屏幕的最大宽换行*/
			{
				x = 0;
				y = y + 16;//每个字符占据16字节
			}
		}
		i++;
	}
	
	
}


