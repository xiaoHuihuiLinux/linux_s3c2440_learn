#include "s3c24xx.h"

/* 用GPIO模拟SPI */

static void SPI_GPIO_Init(void)//可以查看芯片手册spi_jz2440
{
    /* GPF1 OLED_CSn output */
	/*
	00 = Input 01 = Output
	10 = EINT[1] 11 = Reserved
	*/
    GPFCON &= ~(3<<(1*2));//先清掉
    GPFCON |= (1<<(1*2));
    GPFDAT |= (1<<1);//因为片选 OLED_CSn FLASH_CSn同时为0会有影响

    /* GPG2 FLASH_CSn output
    * GPG4 OLED_DC   output
    * GPG5 SPIMISO   input
    * GPG6 SPIMOSI   output
    * GPG7 SPICLK    output
    */
    GPGCON &= ~((3<<(2*2)) | (3<<(4*2)) | (3<<(5*2)) | (3<<(6*2)) | (3<<(7*2)));
    GPGCON |= ((1<<(2*2)) | (1<<(4*2)) | (1<<(6*2)) | (1<<(7*2)));
    GPGDAT |= (1<<2);
}

static void SPI_Set_CLK(char val)//GPG7
{
    if (val)
        GPGDAT |= (1<<7);
    else
        GPGDAT &= ~(1<<7);
}

static void SPI_Set_DO(char val)//SPIMOSI  GPG6
{
    if (val)
        GPGDAT |= (1<<6);//有数据的话将GPG6拉高
    else
        GPGDAT &= ~(1<<6);
}

void SPISendByte(unsigned char val)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        SPI_Set_CLK(0);
        SPI_Set_DO(val & 0x80);//因为发送数据是在sclk的上升沿    而且先发送高位
        SPI_Set_CLK(1);
        val <<= 1;
    }
    
}
static char SPI_Get_DI(void)//GPG5 发出的数据
{
	if(GPGDAT & (1 << 5))
		return 1;//数据是1
	else 
		return 0;
}

unsigned char SPIReceByte(void)
{
    int i;
	unsigned char val =0;
    for (i = 0; i < 8; i++)
    {
    	val <<= 1;
        SPI_Set_CLK(0);
		
       	if(SPI_Get_DI())//对于spi flash 是DO对于2440是DI
			val |= 1;
		SPI_Set_CLK(1);
     }
	return val;
}


void SPIInit(void)
{
    /* 初始化引脚 */
    SPI_GPIO_Init();
}

