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
		//int poll(struct pollfd *fds, nfds_t nfds, int timeout);//nfds_t nfds查询事件个数timeout以毫秒计数
		ret = poll(fds,1,5000);//poll机制意思就是一段时间没有按键的话就返回 指定的时间之内查询
		if(ret == 0)
		{
			printf("time out\n");
		}
		else 
		{
			read(fd,&key_val,1);//第二个参数是指针，在我们用中断 休眠 测试的时候假如没有按键的话就阻塞不会返回我们用poll机制
			printf("key_val = 0x%x\n",key_val);
		}
	}
	return 0;
}



