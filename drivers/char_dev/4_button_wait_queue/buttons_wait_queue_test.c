#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc , char ** argv)
{
	int i;
	int fd;
	int press_cnt[4];
	
	fd = open("/dev/buttons", O_RDWR);  //���豸
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}
	
	while(1)
	{
		read(fd, press_cnt, sizeof(press_cnt)); //��ȡֵ��press_cnt����������ȡ����sizeof(press_cnt)
		
		for(i = 0 ; i<sizeof(press_cnt)/sizeof(press_cnt[0]);i++)
		{
			if(press_cnt[i]) //������´�����Ϊ0����ӡ����
				printf("K%d has been pressed %d times \n", i+1, press_cnt[i]);
		}
		printf("zuse");
	}
}
