/*******************************************
ʹ��linux3.2.81�ں�
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


/* ����/����/ע��һ��platform_device */

static struct resource led_resource[] = {
    [0] = {
        .start = 0x56000050,         // gpfcon�Ĵ�����ʼ��ַ
        .end   = 0x56000050 + 8 - 1,// gpfcon�Ĵ���������ַ
        .flags = IORESOURCE_MEM,    // ��־,��drv(����)�����������־����ȡ��Դ
    },
    [1] = {
        .start = 5,       					// gpfdat
        .end   = 5,
        .flags = IORESOURCE_IRQ,    //��־,��һ���ǵ����жϲ����������־
    }

};

static void led_release(struct device * dev)
{
}


static struct platform_device led_dev = {
    .name         = "myled",   		// �����platform_driver��Ƕ��driver.nameһ��
    .id       = -1,
    .num_resources    = ARRAY_SIZE(led_resource),  // ��Դ��С
    .resource     = led_resource,   //��Դ���飨ǰ�涨��ģ�
    .dev = { 
    	.release = led_release,    // �������ã�����Ϊ�յ�Ҳ����
	},
};

static int led_dev_init(void)
{
	platform_device_register(&led_dev);   //ע���豸
	return 0;
}

static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev); //ж���豸
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");

