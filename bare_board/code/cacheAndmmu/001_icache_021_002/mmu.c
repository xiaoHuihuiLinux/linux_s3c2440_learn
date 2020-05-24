/*
*����һ��ҳ��
*VA��		PA    			 	CB
 *0  	0  					11  (0d��ַ)						
*0x40000000  0x40000000      11��nor������

*0x30000000	0x30000000      11(����sdaram)
*....................
*0x33f00000	0x33f00000      11(����sdaram)

*�Ĵ����ĵ�ַӳ�䣺     0x4800000~0x5B00001C
			0x4800000    0x4800000   00
			............
			0x5B00001C   0x5B00001C  00
*frambuffer��ַ��0x33c00000
*0x33c00000    0x33c00000       00(���ʹ����cache buffer ���ݲ�������ˢ�µ�lcd ��������Ϊ00)
*link address��
*0xB0000000    0x30000000    11��ʹ�ó����sdram����ʾ��ַ��
*/
#define MMU_SECDESC_AP      (3 << 10) //ap������Ȩ����01 ʹ��ap���������� ��������Ϊ11
#define MMU_SECDESC_DOMAIN  (0 <<  5)
#define MMU_SECDESC_NCNB    (0 <<  2)
#define MMU_SECDESC_WB      (3 <<  2)//CB
#define MMU_SECDESC_TYPE    ((1 << 4) | (1 << 1))
#define MMU_SECDESC_FOR_IO (MMU_SECDESC_AP | MMU_SECDESC_DOMAIN |MMU_SECDESC_NCNB |MMU_SECDESC_TYPE) // io ���ڴ�����������Ƿ���CB
#define MMU_SECDESC_FOR_MEM (MMU_SECDESC_AP | MMU_SECDESC_DOMAIN| MMU_SECDESC_WB  |MMU_SECDESC_TYPE)
#define IO  1
#define MEM 0

void create_secdesc(unsigned int *ttb,unsigned int va,unsigned int pa,int io)
{
	int index;
	index = va / 0x100000;//��0��
	if(io)
		ttb[index] =  (pa & 0xfff00000) | MMU_SECDESC_FOR_IO;//��12λʱsection base address
	else 
		ttb[index] =  (pa & 0xfff00000) | MMU_SECDESC_FOR_MEM;//��12λʱsection base address
}
void create_page_table(void)//����ҳ����ǰ������ַ��Ӧ����Ŀ��Ӧ��
{
	/*1.ҳ������
	2.ҳ��ĵĴ�С
	32bitϵͳ
	 0 -4G ��Ŀ�� 4G/1M = 4096
	 ÿ����Ŀ 4 bytes
	 �ܹ� 4096* 4 16 k
	*/
	//ѡ�� 0x32000000(ռ��16kB)
	//ttb taranslation table base
	unsigned int va,pa;
	int index;
	unsigned int * ttb = (unsigned int *)0x32000000;
	/*����va pa ����ҳ����Ŀ*/
	/*2.1for sdram / nor flash*/
	create_secdesc(ttb,0,0,IO);//��0��ַд������cache Ҳ�ܶ����� ����һֱ��Ϊ���ǳɹ��Ķ������ַ��ʹ��caceh ����ʹ��wb������nor����
	/*2.2for sdram / when nor flash*/
	create_secdesc(ttb,0x40000000,0x40000000,MEM);
	/*2.3 for 64mSDram*/
	va = 0x30000000;
	pa = 0x30000000;
	for(;va < 0x34000000; )
	{
		create_secdesc(ttb,va,pa,MEM);
		va += 0x100000;
		pa += 0x100000;//����1M
	}
	/*2.4�Ĵ�����ַ*/
	va =  0x48000000;
	pa =  0x48000000;
	for(;va <= 0x5B000000; )
	{
		create_secdesc(ttb,va,pa,IO);//�Ĵ�����io��ʽӳ��
		va += 0x100000;
		pa += 0x100000;//����1M
	}
	/*2.5�Ĵ�����ַ*/
	create_secdesc(ttb,0x33c00000, 0x33c00000,IO);
	/*2.6linkaddres*/
	create_secdesc(ttb,0xB0000000,0x30000000,MEM);//�����ַ0xB0000000ָ�������ַ0x3000000
}

