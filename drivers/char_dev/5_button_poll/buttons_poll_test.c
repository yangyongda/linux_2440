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
	
	fd = open("/dev/buttons", O_RDWR);  //���豸
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}

	fds[0].fd     = fd;
	fds[0].events = POLLIN; //�����ݿɶ��¼�
	while(1)
	{
		ret = poll(fds, 1, 5000);  //�������5�룬ֻҪ���¼���������¼��᷵�أ����5����û���¼������򷵻�0
		if (ret == 0)
		{
			printf("time out\n");
		}
		else
		{
			read(fd, press_cnt, sizeof(press_cnt)); //��ȡֵ��press_cnt����������ȡ����sizeof(press_cnt)
			for(i = 0 ; i<sizeof(press_cnt)/sizeof(press_cnt[0]);i++)
			{
				if(press_cnt[i]) //������´�����Ϊ0����ӡ����
					printf("K%d has been pressed %d times \n", i+1, press_cnt[i]);
			}
		}
		
	}
}
