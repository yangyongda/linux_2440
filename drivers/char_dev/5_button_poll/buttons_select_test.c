/*
字符设备测试程序
功能：读取各个按键被按下的次数，使用poll去读取事件，当所设定的事件发生
			后该函数会返回该事件值，如果有事件发生则读取数值，如果指定的时间内
			没有事件发生则打印超时信息
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>


int main(int argc , char ** argv)
{
	int i;
	int fd;
	int ret;
	int press_cnt[4];
	fd_set rfds;
	struct timeval time;
	
	//time.tv_sec = 5; //5s
	
	fd = open("/dev/buttons", O_RDWR);  //打开设备
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}


	while(1)
	{
		time.tv_sec = 5; //5s
		time.tv_usec = 0;//select函数会不断修改timeout的值，所以每次循环都应该重新赋值[windows不受此影响]
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		ret = select(fd + 1, &rfds, NULL, NULL, &time);
		if (!ret)
		{
			printf("time out\n");
		}
		else
		{
			read(fd, press_cnt, sizeof(press_cnt)); //读取值到press_cnt缓存区，读取长度sizeof(press_cnt)
			for(i = 0 ; i<sizeof(press_cnt)/sizeof(press_cnt[0]);i++)
			{
				if(press_cnt[i]) //如果按下次数不为0，打印出来
					printf("K%d has been pressed %d times \n", i+1, press_cnt[i]);
			}
		}
		
	}
}
