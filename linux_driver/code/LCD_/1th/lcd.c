#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>
//声明
static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info);

struct  lcd_regs{
	
	unsigned long	lcdcon1;//0X4D000000 lcd 控制1
	unsigned long	lcdcon2;//0X4D000004 lcd 控制2
	unsigned long	lcdcon3;// lcd 控制3
	unsigned long	lcdcon4;// lcd 控制4
	unsigned long	lcdcon5;// lcd 控制5
    unsigned long	lcdsaddr1;//STN/TFT 帧缓冲区开始地址1
    unsigned long	lcdsaddr2;//STN/TFT 帧缓冲区开始地址2
    unsigned long	lcdsaddr3;//STN/TFT 帧缓冲区开始地址3
    unsigned long	redlut;//红色查找表
    unsigned long	greenlut;//绿色查找表
    unsigned long	bluelut;//蓝色查找表 他的地址是0X4D000028
    unsigned long	reserved[9];//为什莫不是差8呢?
    unsigned long	dithmode;//0X4D00004C
    unsigned long	tpal;//TFT临时调色板
    unsigned long	lcdintpnd;//lcd中断等待
    unsigned long	lcdsrcpnd;//lcd中断屏蔽
    unsigned long	lcdintmsk;//lcd中断屏蔽
    unsigned long	lpcsel;
};

static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= s3c_lcdfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,//这三个函数一般都有是别人提供的
};
static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs* lcd_regs;
static u32 pseudo_palette[16];


/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;//设置后的值
	if(regno > 16)
		return 1;
	val = chan_to_field(red, &info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue, &info->var.blue);
	//((u32)(info->pseudo_palette))[regno] = val; //因为pseudo_palette 是 void*型，这是强制转换.这样写比较通用
	pseudo_palette[regno] = val;//因为是设置自已的假调色板，就直接这样写不加 info。
	return 0;
}
static int Lcd_init(void)
{
	/* 1. 分配一个fb_info */
	s3c_lcd = framebuffer_alloc(0, NULL);

	/* 2. 设置 */
	/* 2.1 设置固定的参数 */
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240*320*16/8;
	s3c_lcd->fix.type     = FB_TYPE_PACKED_PIXELS;
	s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR; /* TFT */
	s3c_lcd->fix.line_length = 240*2;
	
	/* 2.2 设置可变的参数 */
	s3c_lcd->var.xres           = 240;
	s3c_lcd->var.yres           = 320;
	s3c_lcd->var.xres_virtual   = 240;
	s3c_lcd->var.yres_virtual   = 320;
	s3c_lcd->var.bits_per_pixel = 16;

	/* RGB:565 */
	s3c_lcd->var.red.offset     = 11;
	s3c_lcd->var.red.length     = 5;
	
	s3c_lcd->var.green.offset   = 5;
	s3c_lcd->var.green.length   = 6;

	s3c_lcd->var.blue.offset    = 0;
	s3c_lcd->var.blue.length    = 5;

	s3c_lcd->var.activate       = FB_ACTIVATE_NOW;
	
	
	/* 2.3 设置操作函数 */
	s3c_lcd->fbops              = &s3c_lcdfb_ops;
	
	/* 2.4 其他的设置 */
	s3c_lcd->pseudo_palette = pseudo_palette;
	//s3c_lcd->screen_base  = ;  /* 显存的虚拟地址 */ 
	s3c_lcd->screen_size   = 240*324*16/8;

	/* 3. 硬件相关的操作 */
	/* 3.1 配置GPIO用于LCD */
	gpbcon = ioremap(0x56000010, 8);
	gpbdat = gpbcon+1;
	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);

    *gpccon  = 0xaaaaaaaa;   /* GPIO管脚用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND */
	*gpdcon  = 0xaaaaaaaa;   /* GPIO管脚用于VD[23:8] */
	*gpbcon &= ~(3);  /* GPB0设置为输出引脚 */
	*gpbcon |= 1;
	*gpbdat &= ~1;     /* 输出低电平 /* 输出低电平.与上 '1 的取反' 即与上0，结果为0，输出低电平 。意思是先不让背光开启.*/ 
	*gpgcon |= (3<<8); /* GPG4用作LCD_PWREN这是LCD 本身的电源。LCD 工作起来后， */
	/* 3.2 根据LCD手册设置LCD控制器, 比如VCLK的频率等 */
	lcd_regs = ioremap(0X4D000000,sizeof(struct lcd_regs));
	/*VCLK = HCLK / [(CLKVAL + 1) × 2  HCLK 100Mhz VCLK  100ns 就是 10mhz
	*bit[17:8]10MHz= 100MHz/ [(CLKVAL + 1) × 2 ----> 最后算得：CLKVAL = 4.
	*MMODE 设置： bit[7]，“决定VM 的触发频率”这个和我们这里设置没什么关系
	*bit[6:5]PNRMODE 设置：bit[6:5]，“选择显示模式” 11 TFT
	*bit[4:1]BPPMODE 设置： bit[4:1] ，是像素的格式 1100 16 bpp
	*ENVID [0]设置： bit[0]，LCD 视频输出和逻辑使能/禁止。设置完后再使能。所
	*/
	lcd_regs->lcdcon1 = (4 << 8) | (3 << 5) | (0x0C << 1);
	/*垂直方面的时间参数。因为电子枪到了最后一行不可能立马跳到第一行需要一个时间
	*VBPD[31:24] 帧同步后，帧数开始之前无效行信号的数量。 VSYNC之后再过多长时间才能发出第1行数据
	 *             LCD手册 T0-T2-T1=4
	*LINEVAL[23:14] 面板垂直的尺寸
	*VFPD[13:6] 帧数据结束后帧同步前无效行信号的数量。发出最后一行数据之后，再过多长时间才发出VSYNC
	 *             LCD手册T2-T5=322-320=2, 所以VFPD=2-1=1
	*VSPW[5:0] 通过计算无效行的数量决定帧同步信号的脉冲高电平的宽度 VSYNC信号的脉冲宽度 LCD手册T1=1, 所以VSPW=1-1=0
	*/
	lcd_regs->lcdcon2 = (3<<24) | (319<<14) | (1<<6) | (0<<0);
	/* 水平方向的时间参数
	 * bit[25:19]: HBPD, VSYNC之后再过多长时间才能发出第1行数据
	 *             LCD手册 T6-T7-T8=17
	 *             HBPD=16
	 * bit[18:8]: 多少列, 240, 所以HOZVAL=240-1=239
	 * bit[7:0] : HFPD, 发出最后一行里最后一个象素数据之后，再过多长时间才发出HSYNC
	 *             LCD手册T8-T11=251-240=11, 所以HFPD=11-1=10
	 */
	lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);
	////4.2.4.4,设置LCD 控制器2:LCDCON4
	/* 水平方向的同步信号
	*MVAL[15:8]
	* bit[7:0] : HSPW, HSYNC 信号的脉冲宽度, LCD 手册
	T7=5, 所以HSPW=5-1=4 ？？？？
	*/
	lcd_regs->lcdcon4 = 4<<0;
	/* 信号的极性 
	 *VSTATUS[16:15] read only
	 *HSTATUS[14:13] read only
	 *BPP24BL[12]This bit determines the order of 24 bpp video memory.24bpp 视频内存的存放顺序24bpp 视频内存的存
	 * bit[11]: 1=565 format
	 * bit[10]: 0 = The video data is fetched at VCLK falling edge
	 * bit[9] : 1 = HSYNC信号要反转,即低电平有效 
	 * bit[8] : 1 = VSYNC信号要反转,即低电平有效 
	 * bit[6] : 0 = VDEN不用反转
	 * bit[3] : 0 = PWREN输出0
	 * bit[1] : 0 = BSWP
	 * bit[0] : 1 = HWSWP 2440手册P 413 在显存中的排布序列
	 */
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);
	/* 3.3 分配显存(framebuffer), 并把地址告诉LCD控制器 */
	//参考 dma_alloc_writecombine(fbi->dev, fbi->map_size,&fbi->map_dma, GFP_KERNEL);
	//参数1:设备 参数2：大小 参数3：实际就是物理地址
	s3c_lcd->screen_base = dma_alloc_writecombine(NULL,s3c_lcd->fix.smem_len,&s3c_lcd->fix.smem_start,GFP_KERNEL);
	//并把地址告诉LCD控制器 
	lcd_regs->lcdsaddr1 = (s3c_lcd->fix.smem_start >> 1) && ~(3<<30);
	lcd_regs->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3 = (240*16/16); //一行的长度(单位: 2 字节)
	//s3c_lcd->fix.smem_start = xxx;  /* 显存的物理地址 */
	//使能
	lcd_regs->lcdcon1 |= (1<<0); //使能LCD 控制器
	lcd_regs->lcdcon5 |= (1<<3); //这个要设置，不然LCD 不显示。这是使能LCD 本身。
	*gpbdat |= 1;//LCD 背光开启
	//注册
	register_framebuffer(s3c_lcd);//实际的底层驱动设备注册之后fbmem 才有意义
	return 0;
}
static void Lcd_exit(void)
{
	//去掉这个结构
	unregister_framebuffer(s3c_lcd);
	//关闭电源
	lcd_regs->lcdcon1 &= ~(1<<0); //不使能LCD 控制器
	*gpbdat &= ~1;//LCD 背光关闭
	//1.2.3，释放掉分配的显存。
	dma_free_writecombine(NULL, s3c_lcd->fix.smem_len,s3c_lcd->screen_base, s3c_lcd->fix.smem_start);
	//1.2.4,iounmap();
	iounmap(lcd_regs);
	iounmap(gpbcon);
	iounmap(gpccon);
	iounmap(gpdcon);
	iounmap(gpgcon);
	//1.2.5,释放 framebuffer 结构体.
	framebuffer_release(s3c_lcd);
}
module_init(Lcd_init);
module_exit(Lcd_exit);

MODULE_LICENSE("GPL");


