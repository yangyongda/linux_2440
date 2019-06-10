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
	/* ����GPF4,5,6Ϊ��� */
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));
	return 0;
}

static ssize_t leds_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	
	//���û��ռ�����ݴ��͵��ں˿ռ�
	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		// ���
		*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	}
	else
	{
		// ���
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}
	
	return 0;
}

/*
��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ��������ջ��������ṹ�����ϵĶ�Ӧ����
*/
static struct file_operations leds_drv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   leds_drv_open,     
		.write	=		leds_drv_write,	   
};


int major;
//ָ��insmod����ʱ������������
static int leds_drv_init(void)
{
	//��һ�������������0�����ʾ����ϵͳ��̬��������豸�ţ���Ϊ0�����ʾ��̬ע��
	major = register_chrdev(0, "leds_drv", &leds_drv_fops); // ע��, �����ںˣ����ض�̬���������豸��
	
	//�������������Ϊ��ʵ���Զ������豸
	ledsdrv_class = class_create(THIS_MODULE, "ledsdrv");  //class_createΪ���豸����һ��class
	ledsdrv_class_dev = device_create(ledsdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/xyz */ //class_device_create������Ӧ���豸
	
	//������Ҫʹ�������ַ������ֱ��ʹ�������ַ
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);  //gpfconӳ��Ϊ0x56000050,ӳ�䳤��Ϊ16���ֽ�(gpfconΪ���ַ,0x56000050Ϊ�����ַ)
	gpfdat = gpfcon + 1;  //gpfcon + 1��Ϊ0x56000050+4

	return 0;
}

//ִ��rmmodʱ������������
static void leds_drv_exit(void)
{
	unregister_chrdev(major, "leds_drv"); 				// ж��

	device_unregister(ledsdrv_class_dev);  //���Զ��������豸ע��
	class_destroy(ledsdrv_class);       				//ɾ����������
	iounmap(gpfcon);              							//ȡ��gpfcon��ӳ��
}

//ָ����������ĳ�ʼ��������ж�غ���
module_init(leds_drv_init);
module_exit(leds_drv_exit);


MODULE_LICENSE("GPL v2");  //��ֹ���֡�module license��unspecified taints kernel�ľ���

