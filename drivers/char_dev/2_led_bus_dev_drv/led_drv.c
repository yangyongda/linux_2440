/*******************************************
使用linux3.8.1内核
********************************************/

/* 分配/设置/注册一个platform_driver */

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

static int major;


static struct class *cls;
static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/* 配置为输出 */
	*gpio_con &= ~(0x3<<(pin*2));
	*gpio_con |= (0x1<<(pin*2));
	return 0;	
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("first_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user(); 从用户层获取数据

	if (val == 1)
	{
		// 点灯
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		// 灭灯
		*gpio_dat |= (1<<pin);
	}
	
	return 0;
}

//供register_chrdev使用
static struct file_operations led_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   led_open,     
		.write	=	led_write,	   
};

/*
device_add和driver_register都会两两比较两者的bus_id或name，
如果一样就调用driver的probe
*/
static int led_probe(struct platform_device *pdev)
{
	struct resource		*res;

	/* 根据platform_device的资源进行ioremap */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);  			//获取标志为IORESOURCE_MEM的资源
	gpio_con = ioremap(res->start, res->end - res->start + 1); //寄存器地址映射为虚拟地址
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);     //获取标志为IORESOURCE_IRQ的资源
	pin = res->start;

	/* 注册字符设备驱动程序 */

	printk("led_probe, found led\n");

	major = register_chrdev(0, "myled", &led_fops);		// 注册, 告诉内核，返回动态创建的主设备号

	cls = class_create(THIS_MODULE, "myled");  				//class_create为该设备创建一个class

	device_create(cls, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */
	
	return 0;
}

static int led_remove(struct platform_device *pdev)
{
	/* 卸载字符设备驱动程序 */
	/* iounmap */
	printk("led_remove, remove led\n");

	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);
	
	return 0;
}

//设置platform_driver
struct platform_driver led_drv = {
	.probe		= led_probe,  //当设备和驱动匹配时会调用该函数
	.remove		= led_remove, //卸载时调用
	.driver		= {
		.name	= "myled",		//必须和platform_device的name一样
	}
};


static int led_drv_init(void)
{
	platform_driver_register(&led_drv);  //注册设备驱动程序
	return 0;
}

static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv); //注销设备驱动
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");



