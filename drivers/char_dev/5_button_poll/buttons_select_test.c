/*
�ַ��豸���Գ���
���ܣ���ȡ�������������µĴ�����ʹ��pollȥ��ȡ�¼��������趨���¼�����
			��ú����᷵�ظ��¼�ֵ��������¼��������ȡ��ֵ�����ָ����ʱ����
			û���¼��������ӡ��ʱ��Ϣ
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
	
	fd = open("/dev/buttons", O_RDWR);  //���豸
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}


	while(1)
	{
		time.tv_sec = 5; //5s
		time.tv_usec = 0;//select�����᲻���޸�timeout��ֵ������ÿ��ѭ����Ӧ�����¸�ֵ[windows���ܴ�Ӱ��]
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		ret = select(fd + 1, &rfds, NULL, NULL, &time);
		if (!ret)
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
