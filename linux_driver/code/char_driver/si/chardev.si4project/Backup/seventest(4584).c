#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<poll.h>
#include<signal.h>
#include<sys/types.h>

#include <unistd.h>
#include <fcntl.h>

int fd;
void my_signal_fun(int signum)
{
	//static int cnt =0;
	//printf("signal =%d, %d times\n",signum,++cnt);
	unsigned char key_val;
	read(fd,&key_val,1);
	printf("key_val: 0x%x\n",key_val);
}
int main(int argc,char **argv)
{
	int Oflags;
	//Ӧ�ó���ע���źŴ�����
	signal(SIGIO,my_signal_fun);//�ں�����SIGIO ��ʾ�����ݶ�д
	fd = open("/dev/buttons",O_RDWR);
	if(fd < 0)
	{
		printf("can't epen\n");
		return -1;
	}
	fcntl(fd, F_SETOWN, getpid());// pid�����ں� ����˭ �ں˰�������
	Oflags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, Oflags | FASYNC); // �ı�fasync��ǣ����ջ���õ�������faync fasync_helpe
	while(1)
	{
		sleep(1000);
	}
	return 0;
}



