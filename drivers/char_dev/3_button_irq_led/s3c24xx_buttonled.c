#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>

static struct class *buttonsdrv_class;   			//类结构体
static struct device	*buttonsdrv_class_dev;  //设备结构体

volatile unsigned long *gpfcon = NULL ;
volatile unsigned long *gpfdat = NULL;

volatile unsigned long *gpgcon = NULL ;
volatile unsigned long *gpgdat = NULL;

unsigned int press_num[3] = {1,2,3};   //按键值
unsigned int pressVal;  //记录按键值


//中断处理函数
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	volatile int *press_num = (volatile int *)dev_id;
		
	pressVal = *press_num;   //记录下按下的键值

    	switch(pressVal)
 	{
 		case 1:
 			*gpfdat |= (1<<4) | (1<<5) | (1<<6);   //全灭
 			*gpfdat &= ~(1<<4);   //点亮第一盏
 			break;
 		case 2:
 			*gpfdat |= (1<<4) | (1<<5) | (1<<6);   //全灭
 			*gpfdat &= ~(1<<5);   //点亮第二盏
 			break;
 		case 3:
 			*gpfdat |= (1<<4) | (1<<5) | (1<<6);   //全灭
 			*gpfdat &= ~(1<<6);   //点亮第三盏
 			break;	
 	}

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

/*
当应用程序操作设备文件时所调用的open、read、write等函数，最终会调用这个结构体中上的对应函数
*/
static struct file_operations buttons_drv_fops = {
    .owner   =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
};


int major;
//指定insmod命令时会调用这个函数
static int buttons_drv_init(void)
{
	//第一个参数如果等于0，则表示采用系统动态分配的主设备号；不为0，则表示静态注册
	major = register_chrdev(0, "buttons_drv", &buttons_drv_fops); // 注册, 告诉内核，返回动态创建的主设备号
	
	//以下两条语句是为了实现自动创建设备
	buttonsdrv_class = class_create(THIS_MODULE, "buttonsdrv");  //class_create为该设备创建一个class
	buttonsdrv_class_dev = device_create(buttonsdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */ //class_device_create创建对应的设备
	
	//驱动中要使用虚拟地址，不能直接使用物理地址
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);  //gpfcon映射为0x56000050,映射长度为16个字节(gpfcon为虚地址,0x56000050为物理地址)
	gpfdat = gpfcon + 1;  //gpfcon + 1即为0x56000050+4

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);  //gpgcon映射为0x56000060,映射长度为16个字节(gpgcon为虚地址,0x560000650为物理地址)
	gpgdat = gpgcon + 1;  //gpgcon + 1即为0x56000060+4
	
/*

	参数解析：中断号，中断处理函数，中断处理标志（如：双边沿触发）,中断名称（任意取）,
						dev_id(中断处理函数)在中断共享时会用到，一般设置为这个设备的设备结构体或者NULL

	*/
	request_irq(IRQ_EINT0,buttons_irq, IRQ_TYPE_EDGE_FALLING, "K2",&press_num[0]); 	/* 配置GPF0为输入引脚 ，注册中断处理函数*/
	request_irq(IRQ_EINT2,buttons_irq, IRQ_TYPE_EDGE_FALLING, "K3",&press_num[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQ_TYPE_EDGE_FALLING, "K4",&press_num[2]);
	
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));  //led引脚配置为输出
	
	return 0;
}

//执行rmmod时会调用这个函数
static void buttons_drv_exit(void)
{
	unregister_chrdev(major, "buttons_drv"); 				// 卸载

	device_unregister(buttonsdrv_class_dev);  //将自动创建的设备注销
	class_destroy(buttonsdrv_class);       				//删除创建的类
	iounmap(gpfcon);              							//取消gpfcon的映射
	iounmap(gpgcon);              							//取消gpgcon的映射
	free_irq(IRQ_EINT0,  &press_num[0]);
	free_irq(IRQ_EINT2,  &press_num[1]);
	free_irq(IRQ_EINT11, &press_num[2]);
}

//指定驱动程序的初始化函数和卸载函数
module_init(buttons_drv_init);
module_exit(buttons_drv_exit);


MODULE_LICENSE("GPL");  //防止出现“module license”unspecified taints kernel的警告

