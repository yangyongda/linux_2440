#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

int main(int argc , char ** argv)
{
	int i;
	int fd;
	int ret;
	int press_cnt[4];
	struct pollfd fds[1];
	
	fd = open("/dev/buttons", O_RDWR);  //打开设备
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}

	fds[0].fd     = fd;
	fds[0].events = POLLIN; //有数据可读事件
	while(1)
	{
		ret = poll(fds, 1, 5000);  //最多阻塞5秒，只要有事件发生则该事件会返回，如果5秒内没有事件发生则返回0
		if (ret == 0)
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
