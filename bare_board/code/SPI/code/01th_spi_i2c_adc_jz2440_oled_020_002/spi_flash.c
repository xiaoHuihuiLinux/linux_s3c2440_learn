/*读出spi flash 的厂家ID */
#include  "s3c24xx.h"
#include  "gpio_spi.h"

static void SPIFLASH_Set_CS(char val)
{
	if(val)
		GPGDAT |=  (1 << 2); //GPG2 spi-flash片选
	else
		GPGDAT &= ~(1 << 2);		
}


static void SPIFlashSendAddr(unsigned int addr)//根据手册发送完90后要发送0字节
{
	SPISendByte(addr >> 16);
	SPISendByte(addr >> 8);
	SPISendByte(addr & 0xFF);
}
void  SPIFLASHReadID(int *PMID,int *PDID)
{
	SPIFLASH_Set_CS(0);/*选中SPI FALSH*/
	SPISendByte(0x90);
	SPIFlashSendAddr(0);
	*PMID = SPIReceByte();//第一个字节是生产ID  对于2440来说是收到
	*PDID = SPIReceByte();//第二个字节是deveceid
	SPIFLASH_Set_CS(1);//选完操作完之后要进行释放
}
