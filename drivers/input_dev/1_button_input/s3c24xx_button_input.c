/*******************************************
ʹ��linux3.8.1�ں�
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
	int irq;   			//�ж�����
	char *name;			//����
	unsigned int pin; 		//����
	unsigned int key_val; 		//����ֵ
};

struct pin_desc pins_desc[4] = {
	{IRQ_EINT0,  "S2", S3C2410_GPF0,   KEY_L},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,   KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3,   KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;       //���水�µİ�����״̬��Ϣ
static struct timer_list buttons_timer;

//�жϴ�����
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;       //���水����Ϣ,��buttons_timer_function����ʹ��
	mod_timer(&buttons_timer, jiffies+HZ/100);//10ms��������ʱ��������������

	return IRQ_RETVAL(IRQ_HANDLED);
}



static void buttons_timer_function(unsigned long data)
{
	struct pin_desc * pindesc = irq_pd;
	unsigned int pinval;

	if (!pindesc)
		return;
	
	pinval = gpio_get_value(pindesc->pin);  //��ȡ����ֵ

	if (pinval)
	{
		/* �ɿ� : ���һ������: 0-�ɿ�, 1-���� */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);  //��������input��ϵͳ���ı����¼�
		input_sync(buttons_dev);    //���������ϲ㣬���ε��¼��Ѿ������.
	}
	else
	{
		/* ���� */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}
}

//ָ��insmod����ʱ������������
static int buttons_drv_init(void)
{
	int i;
	
	/* 1. ����һ��input_dev�ṹ�� */
	buttons_dev = input_allocate_device();
	buttons_dev->name = "keys";

	/* 2. ���� */
	/* 2.1 �ܲ��������¼� */
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);
	
	/* 2.2 �ܲ���������������Щ�¼�: L,S,ENTER,LEFTSHIT */
	set_bit(KEY_L, buttons_dev->keybit); //�����ϵ�L
	set_bit(KEY_S, buttons_dev->keybit); //�����ϵ�S
	set_bit(KEY_ENTER, buttons_dev->keybit);//�����ϵĻس�
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);//�����ϵ���SHIFT

	/* 3. ע�� */
	input_register_device(buttons_dev);
	
	/* 4. Ӳ����صĲ��� */
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);
	
	for (i = 0; i < 4; i++)
	{
		//�����ⲿ�ж�
		request_irq(pins_desc[i].irq, buttons_irq, IRQ_TYPE_EDGE_BOTH, pins_desc[i].name, &pins_desc[i]);
	}
	
	return 0;
}

//ִ��rmmodʱ������������
static void buttons_drv_exit(void)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]); //�ͷ����е��ж�
	}

	del_timer(&buttons_timer);   //ɾ����ʱ��
	input_unregister_device(buttons_dev); //ע���豸
	input_free_device(buttons_dev);	      //�ͷ��豸��Դ
}

//ָ����������ĳ�ʼ��������ж�غ���
module_init(buttons_drv_init);
module_exit(buttons_drv_exit);


MODULE_LICENSE("GPL");  //��ֹ���֡�module license��unspecified taints kernel�ľ���

