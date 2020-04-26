#include "lcd.h"
#include "lcd_controller.h"

#define LCD_NUM 10

static p_lcd_params p_array_lcd[LCD_NUM];
static p_lcd_params g_p_lcd_selected;

int register_lcd(p_lcd_params plcd)
{
	int i;
	for(i = 0;i < LCD_NUM;i++)
	{
		if(!p_array_lcd[i])//Ҳ���� p_array_lcd_params[i]Ϊ�յ�ʱ��Ÿ�ֵ
		{
			p_array_lcd[i] = plcd;
			return i;
		}
	}
	return -1;
}
int select_lcd(char* name)
{
	int i;
	for(i = 0;i < LCD_NUM;i++)
	{
		if(p_array_lcd[i] && !strcmp(p_array_lcd[i]->name, name))
		{
			g_p_lcd_selected = p_array_lcd[i];
			return i;//�ҵ�
		}
	}
	return -1;
}
void get_lcd_params(unsigned int *fb_base,int *xres,int *yres,int *bpp )
{
	*fb_base = g_p_lcd_selected->fb_base;
	*xres = g_p_lcd_selected->xres;
	*yres = g_p_lcd_selected->yres;
	*bpp  = g_p_lcd_selected->bpp;
}
void lcd_enable(void)//�ṩ��lcd.c���ã���װϸ�ڲ��ع���
{
	lcd_controller_enable();
}
void lcd_disable(void)
{
	lcd_controller_disable();
}
int lcd_init(void)
{
	/*ע��lcd*/
	lcd_4_3_add();//���Ѿ����˳�ֵ�Ľṹ����� �����ڽṹ��������

	/* ע��LCD������ */
	lcd_contoller_add();//ͬ���Ľ�lcd�������Ľṹ�屣���ڽṹ��������
	
	/* ѡ��ĳ��LCD */
	select_lcd("lcd_4.3");//��lcd�Ľṹ�����鸳��һ��ȫ�ֱ���
	/*ѡ��ĳ��lcd������*/
	select_lcd_controller("s3c2440");//��lcd�������Ľṹ�����飨��Ҫ���� ������name �ͺ���ָ�룩����һ��ȫ�ֽṹ�����
	/*ʹ��lcd���� ��ʼ��lcd������*/
	lcd_controller_init(g_p_lcd_selected);//lcd �������ĳ�ʼ������ ���� lcd�ṹ�����
}


