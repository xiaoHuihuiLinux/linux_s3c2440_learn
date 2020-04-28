
static unsigned int fb_base;
static int xres,yres,bpp;
extern const unsigned char fontdata_8x16[];

void font_init(void)//��ʼ��
{
	get_lcd_params(&fb_base,&xres,&yres,&bpp);//���ִ��εķ�ʽ�Ƚ��ر�����˼���ں����и��βεĸ�ֵ
}

/*������ĸ�ĵ�����lcd�ϻ�����*/

void fb_print_char(int x,int y,char c,unsigned int color)
{	int i,j;	
	/*����c��assic����fondata����8x16�õ�����*/
	unsigned char * dots = &fontdata_8x16[c * 16]; //������A��Ӧ��ASSICʮ����65��Ӧ����ĵ�65��Ԫ��
	unsigned char data;
	int bit;
	/*���ݵ�����Ⱦ��Ӧ������ɫ*/
	for(j = y;j < y + 16;j++)//�ַ��ĸ�
	{	data = *dots++;
		bit = 7;
		for(i = x;i < x +8;i++)//�ַ��Ŀ�
		{	
			/*���ݵ����ĳλ�Ƿ���ɫ*/
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
			y = y+16;//��������
		}
		else if(str[i] == '\r')
		{
			x = 0;//����
		}
		else
		{
			fb_print_char(x,y,str[i],color);
			x = x+8;
			if(x >= xres)/*��������Ļ��������*/
			{
				x = 0;
				y = y + 16;//ÿ���ַ�ռ��16�ֽ�
			}
		}
		i++;
	}
	
	
}


