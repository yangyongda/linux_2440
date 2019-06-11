/*******************************************
ʹ��linux3.8.1�ں�
********************************************/

/* ����/����/ע��һ��platform_driver */

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
	/* ����Ϊ��� */
	*gpio_con &= ~(0x3<<(pin*2));
	*gpio_con |= (0x1<<(pin*2));
	return 0;	
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("first_drv_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user(); ���û����ȡ����

	if (val == 1)
	{
		// ���
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		// ���
		*gpio_dat |= (1<<pin);
	}
	
	return 0;
}

//��register_chrdevʹ��
static struct file_operations led_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   led_open,     
		.write	=	led_write,	   
};

/*
device_add��driver_register���������Ƚ����ߵ�bus_id��name��
���һ���͵���driver��probe
*/
static int led_probe(struct platform_device *pdev)
{
	struct resource		*res;

	/* ����platform_device����Դ����ioremap */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);  			//��ȡ��־ΪIORESOURCE_MEM����Դ
	gpio_con = ioremap(res->start, res->end - res->start + 1); //�Ĵ�����ַӳ��Ϊ�����ַ
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);     //��ȡ��־ΪIORESOURCE_IRQ����Դ
	pin = res->start;

	/* ע���ַ��豸�������� */

	printk("led_probe, found led\n");

	major = register_chrdev(0, "myled", &led_fops);		// ע��, �����ںˣ����ض�̬���������豸��

	cls = class_create(THIS_MODULE, "myled");  				//class_createΪ���豸����һ��class

	device_create(cls, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */
	
	return 0;
}

static int led_remove(struct platform_device *pdev)
{
	/* ж���ַ��豸�������� */
	/* iounmap */
	printk("led_remove, remove led\n");

	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, "myled");
	iounmap(gpio_con);
	
	return 0;
}

//����platform_driver
struct platform_driver led_drv = {
	.probe		= led_probe,  //���豸������ƥ��ʱ����øú���
	.remove		= led_remove, //ж��ʱ����
	.driver		= {
		.name	= "myled",		//�����platform_device��nameһ��
	}
};


static int led_drv_init(void)
{
	platform_driver_register(&led_drv);  //ע���豸��������
	return 0;
}

static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv); //ע���豸����
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");



