/*
�ַ��豸���Գ���
���ܣ���ȡ�������������µĴ�����ʹ��pollȥ��ȡ�¼��������趨���¼�����
			��ú����᷵�ظ��¼�ֵ��������¼��������ȡ��ֵ�����ָ����ʱ����
			û���¼��������ӡ��ʱ��Ϣ
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

int fd;

//�źŴ�����
void my_signal_fun(int signum)
{
	int i;
	int press_cnt[4];
	
	read(fd, press_cnt, sizeof(press_cnt)); //��ȡֵ��press_cnt����������ȡ����sizeof(press_cnt)
	for(i = 0 ; i<sizeof(press_cnt)/sizeof(press_cnt[0]);i++)
	{
		if(press_cnt[i]) //������´�����Ϊ0����ӡ����
			printf("K%d has been pressed %d times \n", i+1, press_cnt[i]);
	}
}

int main(int argc , char ** argv)
{


	int Oflags;
	
	//��Ӧ�ó����в�׽SIGIO�źţ������������ͣ�  
	signal(SIGIO, my_signal_fun);
	
	fd = open("/dev/buttons", O_RDWR);  //���豸
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}
	
	//����ǰ����PID����Ϊfd�ļ�����Ӧ��������Ҫ����SIGIO,SIGUSR�źŽ���PID  
	fcntl(fd, F_SETOWN, getpid());
	//��ȡfd�Ĵ򿪷�ʽ
	Oflags = fcntl(fd, F_GETFL); //�Ȼ�ȡ��ǰ��״̬��Ȼ�������Ͳ��ᵼ�µ�ǰ״̬��ʧ
	//��fd�Ĵ򿪷�ʽ����ΪFASYNC --- �� ֧���첽֪ͨ  
 //���д���ִ�лᴥ�� ���������� file_operations->fasync ���� ------fasync��������fasync_helper��ʼ��һ��fasync_struct�ṹ�壬�ýṹ�������˽�Ҫ�����źŵĽ���PID (fasync_struct->fa_file->f_owner->pid)  
	fcntl(fd, F_SETFL, Oflags | FASYNC);
	
	
	while (1)
	{
		sleep(5);//�������5��,�źŻ���ǰ�������
		printf("wait!\n");//5������wait! ,һ��Ҫ���ϡ�\n��,�����޷�ˢ�»���������һֱû�����
	}
}
