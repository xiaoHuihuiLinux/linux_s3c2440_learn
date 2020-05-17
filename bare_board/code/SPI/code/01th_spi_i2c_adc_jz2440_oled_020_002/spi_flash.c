/*����spi flash �ĳ���ID */
#include  "s3c24xx.h"
#include  "gpio_spi.h"

static void SPIFLASH_Set_CS(char val)
{
	if(val)
		GPGDAT |=  (1 << 2); //GPG2 spi-flashƬѡ
	else
		GPGDAT &= ~(1 << 2);		
}


static void SPIFlashSendAddr(unsigned int addr)//�����ֲᷢ����90��Ҫ����0�ֽ�
{
	SPISendByte(addr >> 16);
	SPISendByte(addr >> 8);
	SPISendByte(addr & 0xFF);
}
void  SPIFLASHReadID(int *PMID,int *PDID)
{
	SPIFLASH_Set_CS(0);/*ѡ��SPI FALSH*/
	SPISendByte(0x90);
	SPIFlashSendAddr(0);
	*PMID = SPIReceByte();//��һ���ֽ�������ID  ����2440��˵���յ�
	*PDID = SPIReceByte();//�ڶ����ֽ���deveceid
	SPIFLASH_Set_CS(1);//ѡ�������֮��Ҫ�����ͷ�
}
