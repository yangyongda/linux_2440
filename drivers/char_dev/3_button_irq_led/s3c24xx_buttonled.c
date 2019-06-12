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

static struct class *buttonsdrv_class;   			//��ṹ��
static struct device	*buttonsdrv_class_dev;  //�豸�ṹ��

volatile unsigned long *gpfcon = NULL ;
volatile unsigned long *gpfdat = NULL;

volatile unsigned long *gpgcon = NULL ;
volatile unsigned long *gpgdat = NULL;

unsigned int press_num[3] = {1,2,3};   //����ֵ
unsigned int pressVal;  //��¼����ֵ


//�жϴ�����
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	volatile int *press_num = (volatile int *)dev_id;
		
	pressVal = *press_num;   //��¼�°��µļ�ֵ

    	switch(pressVal)
 	{
 		case 1:
 			*gpfdat |= (1<<4) | (1<<5) | (1<<6);   //ȫ��
 			*gpfdat &= ~(1<<4);   //������һյ
 			break;
 		case 2:
 			*gpfdat |= (1<<4) | (1<<5) | (1<<6);   //ȫ��
 			*gpfdat &= ~(1<<5);   //�����ڶ�յ
 			break;
 		case 3:
 			*gpfdat |= (1<<4) | (1<<5) | (1<<6);   //ȫ��
 			*gpfdat &= ~(1<<6);   //��������յ
 			break;	
 	}

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

/*
��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ��������ջ��������ṹ�����ϵĶ�Ӧ����
*/
static struct file_operations buttons_drv_fops = {
    .owner   =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
};


int major;
//ָ��insmod����ʱ������������
static int buttons_drv_init(void)
{
	//��һ�������������0�����ʾ����ϵͳ��̬��������豸�ţ���Ϊ0�����ʾ��̬ע��
	major = register_chrdev(0, "buttons_drv", &buttons_drv_fops); // ע��, �����ںˣ����ض�̬���������豸��
	
	//�������������Ϊ��ʵ���Զ������豸
	buttonsdrv_class = class_create(THIS_MODULE, "buttonsdrv");  //class_createΪ���豸����һ��class
	buttonsdrv_class_dev = device_create(buttonsdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/buttons */ //class_device_create������Ӧ���豸
	
	//������Ҫʹ�������ַ������ֱ��ʹ�������ַ
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);  //gpfconӳ��Ϊ0x56000050,ӳ�䳤��Ϊ16���ֽ�(gpfconΪ���ַ,0x56000050Ϊ�����ַ)
	gpfdat = gpfcon + 1;  //gpfcon + 1��Ϊ0x56000050+4

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);  //gpgconӳ��Ϊ0x56000060,ӳ�䳤��Ϊ16���ֽ�(gpgconΪ���ַ,0x560000650Ϊ�����ַ)
	gpgdat = gpgcon + 1;  //gpgcon + 1��Ϊ0x56000060+4
	
/*

	�����������жϺţ��жϴ��������жϴ����־���磺˫���ش�����,�ж����ƣ�����ȡ��,
						dev_id(�жϴ�����)���жϹ���ʱ���õ���һ������Ϊ����豸���豸�ṹ�����NULL

	*/
	request_irq(IRQ_EINT0,buttons_irq, IRQ_TYPE_EDGE_FALLING, "K2",&press_num[0]); 	/* ����GPF0Ϊ�������� ��ע���жϴ�����*/
	request_irq(IRQ_EINT2,buttons_irq, IRQ_TYPE_EDGE_FALLING, "K3",&press_num[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQ_TYPE_EDGE_FALLING, "K4",&press_num[2]);
	
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));  //led��������Ϊ���
	
	return 0;
}

//ִ��rmmodʱ������������
static void buttons_drv_exit(void)
{
	unregister_chrdev(major, "buttons_drv"); 				// ж��

	device_unregister(buttonsdrv_class_dev);  //���Զ��������豸ע��
	class_destroy(buttonsdrv_class);       				//ɾ����������
	iounmap(gpfcon);              							//ȡ��gpfcon��ӳ��
	iounmap(gpgcon);              							//ȡ��gpgcon��ӳ��
	free_irq(IRQ_EINT0,  &press_num[0]);
	free_irq(IRQ_EINT2,  &press_num[1]);
	free_irq(IRQ_EINT11, &press_num[2]);
}

//ָ����������ĳ�ʼ��������ж�غ���
module_init(buttons_drv_init);
module_exit(buttons_drv_exit);


MODULE_LICENSE("GPL");  //��ֹ���֡�module license��unspecified taints kernel�ľ���

