#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
int main(int argc,char **argv)
{
	int fd;
	int val =1;
	fd = open("/dev/xyz",O_RDWR);
	if(fd < 0)
	{
		printf("can't epen\n");
	}
	if(argc != 2)
	{
		/*
		������Ч��
		Usage :
		./firsttest <on|off>
		*/
		printf("Usage :\n");
		printf("%s <on|off>\n",argv[0]);//<>�ļ����ű�ʾ��������ʡ��
		return 0;
	}
	if(strcmp(argv[1],"on") == 0)
	{
		val =1;		
	}
	else 
	{
		val =0;
	}
	write(fd,&val,4);
	return 0;
}



