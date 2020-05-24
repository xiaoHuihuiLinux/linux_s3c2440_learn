/*
*创建一级页表
*VA　		PA    			 	CB
 *0  	0  					11  (0d地址)						
*0x40000000  0x40000000      11（nor启动）

*0x30000000	0x30000000      11(整个sdaram)
*....................
*0x33f00000	0x33f00000      11(整个sdaram)

*寄存器的地址映射：     0x4800000~0x5B00001C
			0x4800000    0x4800000   00
			............
			0x5B00001C   0x5B00001C  00
*frambuffer地址：0x33c00000
*0x33c00000    0x33c00000       00(如果使用了cache buffer 数据不能立马刷新到lcd 所以设置为00)
*link address：
*0xB0000000    0x30000000    11（使得程序从sdram的启示地址）
*/
#define MMU_SECDESC_AP      (3 << 10) //ap假如域权限是01 使用ap来决定访问 我们设置为11
#define MMU_SECDESC_DOMAIN  (0 <<  5)
#define MMU_SECDESC_NCNB    (0 <<  2)
#define MMU_SECDESC_WB      (3 <<  2)//CB
#define MMU_SECDESC_TYPE    ((1 << 4) | (1 << 1))
#define MMU_SECDESC_FOR_IO (MMU_SECDESC_AP | MMU_SECDESC_DOMAIN |MMU_SECDESC_NCNB |MMU_SECDESC_TYPE) // io 与内存的区别在于是否有CB
#define MMU_SECDESC_FOR_MEM (MMU_SECDESC_AP | MMU_SECDESC_DOMAIN| MMU_SECDESC_WB  |MMU_SECDESC_TYPE)
#define IO  1
#define MEM 0

void create_secdesc(unsigned int *ttb,unsigned int va,unsigned int pa,int io)
{
	int index;
	index = va / 0x100000;//第0项
	if(io)
		ttb[index] =  (pa & 0xfff00000) | MMU_SECDESC_FOR_IO;//高12位时section base address
	else 
		ttb[index] =  (pa & 0xfff00000) | MMU_SECDESC_FOR_MEM;//高12位时section base address
}
void create_page_table(void)//创建页表就是把虚拟地址对应的条目对应好
{
	/*1.页表在哪
	2.页表的的大小
	32bit系统
	 0 -4G 条目数 4G/1M = 4096
	 每个条目 4 bytes
	 总共 4096* 4 16 k
	*/
	//选择 0x32000000(占据16kB)
	//ttb taranslation table base
	unsigned int va,pa;
	int index;
	unsigned int * ttb = (unsigned int *)0x32000000;
	/*根据va pa 设置页表条目*/
	/*2.1for sdram / nor flash*/
	create_secdesc(ttb,0,0,IO);//往0地址写入数据cache 也能读出来 所以一直认为他是成功的对于零地址不使用caceh 不用使用wb否则不能nor启动
	/*2.2for sdram / when nor flash*/
	create_secdesc(ttb,0x40000000,0x40000000,MEM);
	/*2.3 for 64mSDram*/
	va = 0x30000000;
	pa = 0x30000000;
	for(;va < 0x34000000; )
	{
		create_secdesc(ttb,va,pa,MEM);
		va += 0x100000;
		pa += 0x100000;//加上1M
	}
	/*2.4寄存器地址*/
	va =  0x48000000;
	pa =  0x48000000;
	for(;va <= 0x5B000000; )
	{
		create_secdesc(ttb,va,pa,IO);//寄存器以io方式映射
		va += 0x100000;
		pa += 0x100000;//加上1M
	}
	/*2.5寄存器地址*/
	create_secdesc(ttb,0x33c00000, 0x33c00000,IO);
	/*2.6linkaddres*/
	create_secdesc(ttb,0xB0000000,0x30000000,MEM);//虚拟地址0xB0000000指向物理地址0x3000000
}

