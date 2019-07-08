# 输入子系统驱动程序测试
hexdump /dev/input/event0

省略		秒	微秒	类   code   value

0000000 00ac 0000 b539 0007 0001 0026 0001 0000

0000010 00ac 0000 b557 0007 0000 0000 0000 0000

0000020 00ac 0000 3ba9 0009 0001 0026 0000 0000

0000030 00ac 0000 3bc0 0009 0000 0000 0000 0000



这里格式对应的是

struct input_event {

	struct timeval time;

	__u16 type; 

	__u16 code;

	__s32 value;

};
