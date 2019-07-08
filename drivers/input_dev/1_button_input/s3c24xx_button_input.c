/*******************************************
使用linux3.8.1内核
********************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/poll.h>
#include <plat/gpio-cfg.h>

#define S3C2410_GPF0 S3C2410_GPF(0)
#define S3C2410_GPF2 S3C2410_GPF(2)
#define S3C2410_GPG3 S3C2410_GPG(3)
#define S3C2410_GPG11 S3C2410_GPG(11)

struct pin_desc{
	int irq;   			//中断类型
	char *name;			//名称
	unsigned int pin; 		//引脚
	unsigned int key_val; 		//按键值
};

struct pin_desc pins_desc[4] = {
	{IRQ_EINT0,  "S2", S3C2410_GPF0,   KEY_L},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,   KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3,   KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;       //保存按下的按键的状态信息
static struct timer_list buttons_timer;

//中断处理函数
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;       //保存按键信息,供buttons_timer_function函数使用
	mod_timer(&buttons_timer, jiffies+HZ/100);//10ms后启动定时器，类似于消抖

	return IRQ_RETVAL(IRQ_HANDLED);
}



static void buttons_timer_function(unsigned long data)
{
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if (!pindesc)
		return;
	
	pinval = gpio_get_value(pindesc->pin);  //获取引脚值

	if (pinval)
	{
		/* 松开 : 最后一个参数: 0-松开, 1-按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);  //驱动层向input子系统核心报告事件
		input_sync(buttons_dev);    //用来告诉上层，本次的事件已经完成了.
	}
	else
	{
		/* 按下 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}
}

//指定insmod命令时会调用这个函数
static int buttons_drv_init(void)
{
	int i;
	
	/* 1. 分配一个input_dev结构体 */
	buttons_dev = input_allocate_device();
	buttons_dev->name = "keys";

	/* 2. 设置 */
	/* 2.1 能产生哪类事件 */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);
	
	/* 2.2 能产生这类操作里的哪些事件: L,S,ENTER,LEFTSHIT */
	set_bit(KEY_L, buttons_dev->keybit); //键盘上的L
	set_bit(KEY_S, buttons_dev->keybit); //键盘上的S
	set_bit(KEY_ENTER, buttons_dev->keybit);//键盘上的回车
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);//键盘上的左SHIFT

	/* 3. 注册 */
	input_register_device(buttons_dev);
	
	/* 4. 硬件相关的操作 */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);
	
	for (i = 0; i < 4; i++)
	{
		//配置外部中断
		request_irq(pins_desc[i].irq, buttons_irq, IRQ_TYPE_EDGE_BOTH, pins_desc[i].name, &pins_desc[i]);
	}
	
	return 0;
}

//执行rmmod时会调用这个函数
static void buttons_drv_exit(void)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]); //释放所有的中断
	}

	del_timer(&buttons_timer);   //删除定时器
	input_unregister_device(buttons_dev); //注销设备
	input_free_device(buttons_dev);	      //释放设备资源
}

//指定驱动程序的初始化函数和卸载函数
module_init(buttons_drv_init);
module_exit(buttons_drv_exit);


MODULE_LICENSE("GPL");  //防止出现“module license”unspecified taints kernel的警告

