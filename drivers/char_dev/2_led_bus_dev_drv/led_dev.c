/*******************************************
使用linux3.2.81内核
********************************************/
#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>


/* 分配/设置/注册一个platform_device */

static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,         // gpfcon寄存器开始地址
        .end   = 0x56000050 + 8 - 1,// gpfcon寄存器结束地址
        .flags = IORESOURCE_MEM,    // 标志,在drv(驱动)中利用这个标志来获取资源
    },
    [1] = {
        .start = 5,       					// gpfdat
        .end   = 5,
        .flags = IORESOURCE_IRQ,    //标志,不一定非得是中断才能用这个标志
    }

};

static void led_release(struct device * dev)
{
}


static struct platform_device led_dev = {
    .name         = "myled",   		// 必须和platform_driver内嵌的driver.name一样
    .id       = -1,
    .num_resources    = ARRAY_SIZE(led_resource),  // 资源大小
    .resource     = led_resource,   //资源数组（前面定义的）
    .dev = { 
    	.release = led_release,    // 必须设置，函数为空的也可以
	},
};

static int led_dev_init(void)
{
	platform_device_register(&led_dev);   //注册设备
	return 0;
}

static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev); //卸载设备
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");

