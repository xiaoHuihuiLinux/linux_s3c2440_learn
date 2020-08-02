#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
int main(int argc,char **argv)
{
	int fd;
	int val =1;
	unsigned char key_val;
	int cnt;
	fd = open("/dev/buttons",O_RDWR);
	if(fd < 0)
	{
		printf("can't epen\n");
	}
	while(1)
	{
		read(fd,&key_val,1);//第二个参数时指针
		printf("key_val = 0x%x\n",key_val);
	}
	return 0;
}



