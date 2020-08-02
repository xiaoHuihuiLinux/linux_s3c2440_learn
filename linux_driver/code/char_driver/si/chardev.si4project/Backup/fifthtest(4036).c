#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<poll.h>
int main(int argc,char **argv)
{
	int fd;
	struct pollfd fds[1];
	int val =1;
	unsigned char key_val;
	int cnt;
	int ret = 0;;
	fd = open("/dev/buttons",O_RDWR);
	if(fd < 0)
	{
		printf("can't epen\n");
	}
	fds[0].fd = fd;
	fds[0].events =  POLLIN;//There is data to read.
	while(1)
	{
		//int poll(struct pollfd *fds, nfds_t nfds, int timeout);//nfds_t nfds��ѯ�¼�����timeout�Ժ������
		ret = poll(fds,1,5000);//poll������˼����һ��ʱ��û�а����Ļ��ͷ��� ָ����ʱ��֮�ڲ�ѯ
		if(ret == 0)
		{
			printf("time out\n");
		}
		else 
		{
			read(fd,&key_val,1);//�ڶ���������ָ�룬���������ж� ���� ���Ե�ʱ�����û�а����Ļ����������᷵��������poll����
			printf("key_val = 0x%x\n",key_val);
		}
	}
	return 0;
}



