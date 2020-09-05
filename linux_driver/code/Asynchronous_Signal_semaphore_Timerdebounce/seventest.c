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
	int ret;

	unsigned char key_val;
	//应用程序注册信号处理函数
	//signal(SIGIO,my_signal_fun);//内核中用SIGIO 表示有数据读写
	fd = open("/dev/buttons",O_RDWR);
	//fd = open("/dev/buttons",O_RDWR |O_NONBLOCK); //非阻塞的方式我们驱动程序也要做相应的处理
	if(fd < 0)
	{
		printf("can't epen\n");
		return -1;
	}
	//fcntl(fd, F_SETOWN, getpid());// pid告诉内核 发给谁 内核帮我们做
	//Oflags = fcntl(fd, F_GETFL);
	//fcntl(fd, F_SETFL, Oflags | FASYNC); // 改变fasync标记，最终会调用到驱动的faync fasync_helpe
	while(1)//如果我们用阻塞的方式的话只有按键按下才会打印，不按下的话会休眠 非阻塞的话他会返回继续打印
	{
		ret = read(fd,&key_val,1);
		
		printf("key_val: 0x%x,ret = %d\n",key_val,ret);
		
		//sleep(5);
	}
	return 0;
}



