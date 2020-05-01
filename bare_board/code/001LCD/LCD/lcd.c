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
		if(!p_array_lcd[i])//也就是 p_array_lcd_params[i]为空的时候才赋值
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
			return i;//找到
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
void lcd_enable(void)//提供给lcd.c调用，封装细节不必关心
{
	lcd_controller_enable();
}
void lcd_disable(void)
{
	lcd_controller_disable();
}
int lcd_init(void)
{
	/*注册lcd*/
	lcd_4_3_add();//将已经有了初值的结构体变量 保存在结构体数组中

	/* 注册LCD控制器 */
	lcd_contoller_add();//同样的将lcd控制器的结构体保存在结构体数组中
	
	/* 选择某款LCD */
	select_lcd("lcd_4.3");//将lcd的结构体数组赋给一个全局变量
	/*选择某款lcd控制器*/
	select_lcd_controller("s3c2440");//将lcd控制器的结构体数组（主要包括 控制器name 和函数指针）赋给一个全局结构体变量
	/*使用lcd参数 初始化lcd控制器*/
	lcd_controller_init(g_p_lcd_selected);//lcd 控制器的初始化函数 接受 lcd结构体参数
}


