
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* led_test on
  * led_test off
  */
int main(int argc, char **argv)
{
	int fd;   //文件句柄
	int val = 1;
	fd = open("/dev/led", O_RDWR);  //使用读写模式打开/dev/xyz设备文件
	if (fd < 0)
	{
		printf("can't open!\n");  //打开失败
	}
	if (argc != 2)   //参数个数不为2，则显示使用方法
	{
		printf("Usage :\n");
		printf("%s <on|off>\n", argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "on") == 0)   //第二个参数为"on"
	{
		val  = 1;
	}
	else
	{
		val = 0;
	}
	
	write(fd, &val, 4);   //写入4个字节数据到设备文件,该函数最终将调用底层驱动的write函数
	return 0;
}
