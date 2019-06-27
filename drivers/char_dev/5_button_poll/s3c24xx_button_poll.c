/*******************************************
ʹ��linux3.2.81�ں�
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
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/poll.h>

static struct class *buttonsdrv_class;   			//��ṹ��
static struct device	*buttonsdrv_class_dev;  //�豸�ṹ��

volatile unsigned long *gpfcon = NULL ;
volatile unsigned long *gpfdat = NULL;

volatile unsigned long *gpgcon = NULL ;
volatile unsigned long *gpgdat = NULL;

unsigned int press_cnt[4];   //��¼�ð������µĴ���

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);    //����(����)�ȴ�����

/* �ж��¼���־, �жϷ����������1��buttons_drv_read������0 */
static volatile int ev_press = 0;

//�жϴ�����
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
		volatile int *press_cnt = (volatile int *)dev_id;
		
		*press_cnt = *press_cnt +1;  //��������ֵ��һ

    ev_press = 1;                  /* ��ʾ�жϷ����� */
    wake_up_interruptible(&button_waitq);   /* �������ߵĽ��� */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static int buttons_drv_open(struct inode *inode, struct file *file)
{
	/*
	�����������жϺţ��жϴ��������жϴ����־���磺˫���ش�����,�ж����ƣ�����ȡ��,
						dev_id(�жϴ�����)���жϹ���ʱ���õ���һ������Ϊ����豸���豸�ṹ�����NULL
	*/
	request_irq(IRQ_EINT0,buttons_irq, IRQ_TYPE_EDGE_BOTH, "S2",&press_cnt[0]); 	/* ����GPF0Ϊ�������� ��ע���жϴ�����*/
	request_irq(IRQ_EINT2,buttons_irq, IRQ_TYPE_EDGE_BOTH, "S3",&press_cnt[1]);
	request_irq(IRQ_EINT11,buttons_irq,IRQ_TYPE_EDGE_BOTH, "S4",&press_cnt[2]);
	request_irq(IRQ_EINT19,buttons_irq,IRQ_TYPE_EDGE_BOTH, "S5",&press_cnt[3]);
	
	return 0;
}


static int buttons_drv_close(struct inode *inode, struct file *file)
{
	//�ͷ��Ѿ�ע����ж�
	free_irq(IRQ_EINT0,  &press_cnt[0]);
	free_irq(IRQ_EINT2,  &press_cnt[1]);
	free_irq(IRQ_EINT11, &press_cnt[2]);
	free_irq(IRQ_EINT19, &press_cnt[3]);
	return 0;
}


static int buttons_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{

	/* ���û�а�������, ����     wait_event_interruptible�ĵڶ���������һ���ж���������������������˳����ߣ������������*/
	wait_event_interruptible(button_waitq, ev_press); //���������жϴ������лὫev_press=1

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, press_cnt, min(sizeof(press_cnt),size));  //����ʵ����Ҫ������Ӧ������ֵ���û��ռ�
	ev_press = 0;   //��ȡ�����0
	
	return 1;
}


static unsigned buttons_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); //�ѵȴ�����׷�ӵ��ں˹���Ľ��̵� poll_table�� ������������

	if (ev_press)  //�����жϷ������жϺ����Ὣev_press��һ
		mask |= POLLIN | POLLRDNORM;  //POLLIN | POLLRDNORM ��ʾ�ɶ�

	return mask;
}

/*
��Ӧ�ó�������豸�ļ�ʱ�����õ�open��read��write�Ⱥ��������ջ��������ṹ�����ϵĶ�Ӧ����
*/
static struct file_operations buttons_drv_fops = {
    .owner   =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open    =   buttons_drv_open, 
    .release =   buttons_drv_close,  
		.read	   =	 buttons_drv_read,	   
		.poll		 =	 buttons_drv_poll,
};


int major;
//ָ��insmod����ʱ������������
static int buttons_drv_init(void)
{
	//��һ�������������0�����ʾ����ϵͳ��̬��������豸�ţ���Ϊ0�����ʾ��̬ע��
	major = register_chrdev(0, "buttons_drv", &buttons_drv_fops); // ע��, �����ںˣ����ض�̬���������豸��
	
	//�������������Ϊ��ʵ���Զ������豸
	buttonsdrv_class = class_create(THIS_MODULE, "buttonsdrv");  //class_createΪ���豸����һ��class
	buttonsdrv_class_dev = device_create(buttonsdrv_class, NULL, MKDEV(major, 0), NULL, "buttons"); /* /dev/xyz */ //class_device_create������Ӧ���豸
	
	//������Ҫʹ�������ַ������ֱ��ʹ�������ַ
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);  //gpfconӳ��Ϊ0x56000050,ӳ�䳤��Ϊ16���ֽ�(gpfconΪ���ַ,0x56000050Ϊ�����ַ)
	gpfdat = gpfcon + 1;  //gpfcon + 1��Ϊ0x56000050+4

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);  //gpgconӳ��Ϊ0x56000060,ӳ�䳤��Ϊ16���ֽ�(gpgconΪ���ַ,0x560000650Ϊ�����ַ)
	gpgdat = gpgcon + 1;  //gpgcon + 1��Ϊ0x56000060+4
	
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
}

//ָ����������ĳ�ʼ��������ж�غ���
module_init(buttons_drv_init);
module_exit(buttons_drv_exit);


MODULE_LICENSE("GPL");  //��ֹ���֡�module license��unspecified taints kernel�ľ���

