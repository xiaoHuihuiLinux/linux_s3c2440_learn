
void do_write_at24cxx(void)
{
	unsigned int addr;
	unsigned char str[100];
	int err;
	
	/* 获得地址 */
	printf("Enter the address of sector to write: ");
	addr = get_uint();

	if (addr > 256)
	{
		printf("address > 256, error!\n\r");
		return;
	}

	printf("Enter the string to write: ");
	gets(str);

	printf("writing ...\n\r");
	err = at24cxx_write(addr, str, strlen(str)+1);
	printf("at24cxx_write ret = %d\n\r", err);
}
void do_read_at24cxx(void)
{
	unsigned int addr;
	int i, j;
	unsigned char c;
	unsigned char data[100];
	unsigned char str[16];
	int len;
	int err;
	int cnt = 0;
	
	/* 获得地址 */
	printf("Enter the address to read: ");
	addr = get_uint();
	
	if(addr > 256)
	{
		printf("address > 256, error!\n\r");
		return;
	}

	/* 获得长度 */
	printf("Enter the length to read: ");
	len = get_int();
	/*因为的形参是指针所以后面的数据会保存到data中
	因为最后在中断中将寄存器的值赋给p_cur_msg这个全局变量的buf成员
	p_cur_msg = msg在读写的时候msg的地址都指向了p_cur_msg的 地址所以data就是所读到的数据*/

	err = at24cxx_read(addr, data, len);//要读的地址 存放的位置  长度
	printf("at24cxx_read ret = %d\n\r",err);


	printf("Data : \n\r");
	/* 长度固定为64 */
	for (i = 0; i < 4; i++)
	{
		/* 每行打印16个数据 */
		for (j = 0; j < 16; j++)
		{
			/* 先打印数值 */
			c = data[cnt++];
			str[j] = c;
			printf("%02x ", c);
		}

		printf("   ; ");

		for (j = 0; j < 16; j++)
		{
			/* 后打印字符 */
			if (str[j] < 0x20 || str[j] > 0x7e)  /* 不可视字符 */
				putchar('.');
			else
				putchar(str[j]);
		}
		printf("\n\r");
	}
}

void i2c_test(void)
{
	char c;
	/*初始化*/
	i2c_init();
	while (1)
	{
		/* 打印菜单, 供我们选择测试内容 */
		printf("[w] Write at24cxx\n\r");
		printf("[r] Read at24cxx\n\r");
		printf("[q] quit\n\r");
		printf("Enter selection: ");

		c = getchar();//从串口得到数据
		printf("%c\n\r", c);//打印出来

		/* 测试内容:
		 * 1. 识别nor flash
		 * 2. 擦除nor flash某个扇区
		 * 3. 编写某个地址
		 * 4. 读某个地址
		 */
		switch (c)		 
		{
			case 'q':
			case 'Q':
				return;
				break;
				
			case 'w':
			case 'W':
				do_write_at24cxx();
				break;

			case 'r':
			case 'R':
				do_read_at24cxx();
				break;
			default:
				break;
		}
	}
}


