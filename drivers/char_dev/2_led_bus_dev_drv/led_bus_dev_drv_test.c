
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* led_test on
  * led_test off
  */
int main(int argc, char **argv)
{
	int fd;   //�ļ����
	int val = 1;
	fd = open("/dev/led", O_RDWR);  //ʹ�ö�дģʽ��/dev/xyz�豸�ļ�
	if (fd < 0)
	{
		printf("can't open!\n");  //��ʧ��
	}
	if (argc != 2)   //����������Ϊ2������ʾʹ�÷���
	{
		printf("Usage :\n");
		printf("%s <on|off>\n", argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "on") == 0)   //�ڶ�������Ϊ"on"
	{
		val  = 1;
	}
	else
	{
		val = 0;
	}
	
	write(fd, &val, 4);   //д��4���ֽ����ݵ��豸�ļ�,�ú������ս����õײ�������write����
	return 0;
}
