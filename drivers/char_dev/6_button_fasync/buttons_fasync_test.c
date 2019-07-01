/*
字符设备测试程序
功能：读取各个按键被按下的次数，使用poll去读取事件，当所设定的事件发生
			后该函数会返回该事件值，如果有事件发生则读取数值，如果指定的时间内
			没有事件发生则打印超时信息
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

int fd;

//信号处理函数
void my_signal_fun(int signum)
{
	int i;
	int press_cnt[4];
	
	read(fd, press_cnt, sizeof(press_cnt)); //读取值到press_cnt缓存区，读取长度sizeof(press_cnt)
	for(i = 0 ; i<sizeof(press_cnt)/sizeof(press_cnt[0]);i++)
	{
		if(press_cnt[i]) //如果按下次数不为0，打印出来
			printf("K%d has been pressed %d times \n", i+1, press_cnt[i]);
	}
}

int main(int argc , char ** argv)
{


	int Oflags;
	
	//在应用程序中捕捉SIGIO信号（由驱动程序发送）  
	signal(SIGIO, my_signal_fun);
	
	fd = open("/dev/buttons", O_RDWR);  //打开设备
	if(fd<0)
	{
		printf("Can't open");
		return -1;
	}
	
	//将当前进程PID设置为fd文件所对应驱动程序将要发送SIGIO,SIGUSR信号进程PID  
	fcntl(fd, F_SETOWN, getpid());
	//获取fd的打开方式
	Oflags = fcntl(fd, F_GETFL); //先获取当前的状态，然后或运算就不会导致当前状态丢失
	//将fd的打开方式设置为FASYNC --- 即 支持异步通知  
 //该行代码执行会触发 驱动程序中 file_operations->fasync 函数 ------fasync函数调用fasync_helper初始化一个fasync_struct结构体，该结构体描述了将要发送信号的进程PID (fasync_struct->fa_file->f_owner->pid)  
	fcntl(fd, F_SETFL, Oflags | FASYNC);
	
	
	while (1)
	{
		sleep(5);//挂起进程5秒,信号会提前打断休眠
		printf("wait!\n");//5秒后输出wait! ,一定要加上“\n”,否则无法刷新缓存区，会一直没有输出
	}
}
