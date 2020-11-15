#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/* i2c_test r addr
 * i2c_test w addr val
 */

void print_usage(char *file)
{
	printf("%s r addr\n",file);
	printf("%s w addr val\n", file);
}
int main(int argc,char **argv)//因为第二个参数是char **argv  *argv 认为是数组
{
	int fd;
	unsigned char buf[2];
	if( (argc != 3) && (argc != 4))
	{
		print_usage(argv[0]);//所以我们在实现子函数的时候形参要是 char *
		return -1;
	}
	fd = open("/dev/at24cxx",O_RDWR);
	if(fd < 0)
	{
		printf("can't open /dev/at24cxx\n");
		return -1;
	}
	if(strcmp(argv[1],"r") == 0)
	{
		/*
		首先：将我们要写的地址转换为一个数字放在buf[0]
		然后调用read()函数，在read的底层at24cxx_read()我们首先将buf[0]的地址拿出来
		发给 at24cxx_client->addr这个地址 
		之后才能将at24cxx_client->addr 地址对应的数据拿出来。也就是那出我们要读的某个地址
		*/
		buf[0] = strtoul(argv[2],NULL,0);//将命令行输入的地址转换为数字
		read(fd,buf,1);//
		printf("data: %c,%d,0x%2x\n",buf[0],buf[0],buf[0]);//以字符正数十六进制分别打印出来
	}
	else if(strcmp(argv[1],"w") == 0)
	{
		buf[0] = strtoul(argv[2], NULL, 0);
		buf[1] = strtoul(argv[3], NULL, 0);
		write(fd, buf, 2);
	}
	else 
	{
		print_usage(argv[0]);
		return -1;
	}
	return 0;
	
}
