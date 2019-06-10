#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>

static struct class *ledsdrv_class;
static struct device	*ledsdrv_class_dev;

volatile unsigned long *gpfcon = NULL ;
volatile unsigned long *gpfdat = NULL;


static int leds_drv_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/* 配置GPF4,5,6为输出 */
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));
	return 0;
}

static ssize_t leds_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	
	//将用户空间的数据传送到内核空间
	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// 点灯
		*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	}
	else
	{
		// 灭灯
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}
	
	return 0;
}

/*
当应用程序操作设备文件时所调用的open、read、write等函数，最终会调用这个结构体中上的对应函数
*/
static struct file_operations leds_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   leds_drv_open,     
		.write	=		leds_drv_write,	   
};


int major;
//指定insmod命令时会调用这个函数
static int leds_drv_init(void)
{
	//第一个参数如果等于0，则表示采用系统动态分配的主设备号；不为0，则表示静态注册
	major = register_chrdev(0, "leds_drv", &leds_drv_fops); // 注册, 告诉内核，返回动态创建的主设备号
	
	//以下两条语句是为了实现自动创建设备
	ledsdrv_class = class_create(THIS_MODULE, "ledsdrv");  //class_create为该设备创建一个class
	ledsdrv_class_dev = device_create(ledsdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/xyz */ //class_device_create创建对应的设备
	
	//驱动中要使用虚拟地址，不能直接使用物理地址
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);  //gpfcon映射为0x56000050,映射长度为16个字节(gpfcon为虚地址,0x56000050为物理地址)
	gpfdat = gpfcon + 1;  //gpfcon + 1即为0x56000050+4

	return 0;
}

//执行rmmod时会调用这个函数
static void leds_drv_exit(void)
{
	unregister_chrdev(major, "leds_drv"); 				// 卸载

	device_unregister(ledsdrv_class_dev);  //将自动创建的设备注销
	class_destroy(ledsdrv_class);       				//删除创建的类
	iounmap(gpfcon);              							//取消gpfcon的映射
}

//指定驱动程序的初始化函数和卸载函数
module_init(leds_drv_init);
module_exit(leds_drv_exit);


MODULE_LICENSE("GPL v2");  //防止出现“module license”unspecified taints kernel的警告

