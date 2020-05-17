#include "i2c_controller.h"
#include "../s3c2440_soc.h"
static p_i2c_msg p_cur_msg;//���ڱ��浱ǰ�Ĵ�������
int isLastData(void)
{
	if(p_cur_msg->cnt_transferred  == p_cur_msg->len -1)//��Ҫ�������һ���ֽ�
		return 1;/*��Ҫ�������һ������*/
	else 
		return 0;
}
void resume_iic_with_ack(void)
{
	unsigned int iiccon = IICCON;
	iiccon |=   (1 << 7);/*��ӦACK*/
	iiccon &= 	~(1 << 4);/*�ָ�IIC����*/
	IICCON = iiccon;
}
void resume_iic_without_ack(void)
{
	unsigned int iiccon = IICCON;
	
	iiccon &= 	~((1 <<7 ) | (1 << 4));/*����Ӧack �ָ�IIC����*/
	IICCON = iiccon;
}

void i2c_interrupt_func(int irq)//ÿ����һ���ֽھͲ����ж�
{
	int index;
	unsigned int iicstat = IICSTAT;
	unsigned int iiccon;
	p_cur_msg->cnt_transferred++;
	/*ÿ����һ�����ݽ�����һ���ж�*/
	/*����ÿ�δ��� ��һ���ж��ǡ��Ѿ��������豸��ַ��*/
	if(p_cur_msg->flags == 0)//write
	{
		/*���ڵ�1���ж�ʱ���ͳ��豸��ַ�������
		��Ҫ�ж�ACK
		��ACK �豸����
		��ACK:���豸������ֱ�Ӵ������
		�������жϼ�����������*/
		if(p_cur_msg->cnt_transferred == 0)//��ʾ��һ���жϣ���ʱ��ô�����ݰ�
		{
			if(iicstat &  (1 << 0))//1��ʾû���յ�ack
			{
				IICSTAT = 0xD0;//�����ֲ�
				IICCON &= ~(1 << 4);
				p_cur_msg->err = -1;//�д���
				delay(1000);
				return;
			}
		}
		if(p_cur_msg->cnt_transferred < p_cur_msg->len)
		{
			/*���������ж�Ҫ������һ������*/
			IICDS = p_cur_msg->buf[p_cur_msg->cnt_transferred];
			IICCON &= ~(1 << 4);
		}
		else 
		{
			IICSTAT = 0xD0;//�����ֲ�
			IICCON &= ~(1 << 4);
			//p_cur_msg->err = -1;//��������
			delay(1000);
		}
		
	}
	else /*read*/
	{
		/*���ڵ�1���ж�ʱ���ͳ��豸��ַ�������
		��Ҫ�ж�ACK
		��ACK �豸���� �ָ�i2c����
		��ACK:���豸������ֱ�Ӵ������
		*/
		/*�ǵ�һ���жϣ���ʾ�õ���һ���µ�����*/
		/*��IIcds��������*/
		if(p_cur_msg->cnt_transferred == 0)//��ʾ��һ���жϣ���ʱ��ô�����ݰ�
		{
			if(iicstat &  (1 << 0))//1��ʾû���յ�ack
			{
				IICSTAT = 0x90;//�����ֲ�
				IICCON &= ~(1 << 4);//���pengding
				p_cur_msg->err = -1;//�д���
				delay(1000);
				return;
			}
			else /*ack*/
			{
				/* ��������һ������, ��������ʱҪ����Ϊ����ӦACK */
				/* �ָ�I2C���� */
				if (isLastData())//��Ҫ�������һ������
				{
					resume_iic_without_ack();
				}
				else 
				{
					resume_iic_with_ack();
				}
				return;
			}
		}
		/*�ǵ�һ���жϱ�ʾ�õ���һ��������*/
		/*��iicds��������*/
		if(p_cur_msg->cnt_transferred < p_cur_msg->len )
		{
			index = p_cur_msg->cnt_transferred -1;//��һ���жϽ��������һ��
			p_cur_msg->buf[index] = IICDS;

			/* ��������һ������, ��������ʱҪ����Ϊ����ӦACK */
			/* �ָ�I2C���� */
			if (isLastData())
			{
				resume_iic_without_ack();
			}
			else 
			{
				resume_iic_with_ack();
			}
		}
		else //ȫ������
		{
			//����ֹͣ�ź�
			IICSTAT = 0x90;
			IICCON &= ~(1 << 4);
			delay(1000);
		}
		
	}
}


void s3c2440_i2c_con_init(void)
{
	/* ������������I2C���򲻻�����ж�*/
	GPECON &= ~((3<<28) | (3<<30));
	GPECON |= ((2<<28) | (2<<30));
	
	/* ����ʱ�� */
	/* [7] : IIC-bus acknowledge enable bit, 1-enable in rx mode
	 * [6] : ʱ��Դ, 0: IICCLK = fPCLK /16; 1: IICCLK = fPCLK /512
	 * [5] : 1-enable interrupt
	 * [4] : ����Ϊ1ʱ��ʾ�жϷ�����, д��0��������ָ�I2C����
	 * [3:0] : Tx clock = IICCLK/(IICCON[3:0]+1).
	 * Tx Clock = 100khz = 50Mhz/16/(IICCON[3:0]+1)
	 */
	IICCON = (1<<7) | (0<<6) | (1<<5) | (30<<0);

	/* ע���жϴ����� */
	register_irq(27, i2c_interrupt_func);
}
int do_master_tx(p_i2c_msg msg)//�����ֲ������ͼ��
{
	p_cur_msg =  msg;//ÿ�δ����ʱ��Ҫ����ĵ�msg ������ǰ�����ȫ�ֱ���
	msg->cnt_transferred = -1;//����ĸ���  Ҳ����ÿ��i2c_msg�ṹ�������len�ļ���Ҳ���Ǵ�����ֽ���
	msg->err = 0;
	
	/* ���üĴ����������� */
	/* 1. ����Ϊ master tx mode */
	IICCON |= (1<<7); /* TX mode, ��ACK�����ͷ�SDA */
	IICSTAT = (1<<4);
		
	/* 2. �Ѵ��豸��ַд��IICDS */
	IICDS = msg->addr<<1;
	
	/* 3. IICSTAT = 0xf0 , ���ݼ������ͳ�ȥ, �������жϲ��� */
	IICSTAT = 0xf0;
	

	/* �����Ĵ������ж����� */

	/* ѭ���ȴ��жϴ������ */
	while (!msg->err && msg->cnt_transferred != msg->len);

	if (msg->err)
		return -1;
	else 
		return 0;
	
}
int do_master_rx(p_i2c_msg msg)
{
	p_cur_msg = msg;

	msg->cnt_transferred = -1;
	msg->err = 0;
	
	/* ���üĴ����������� */
	/* 1. ����Ϊ Master Rx mode */
	IICCON |= (1<<7); /* RX mode, ��ACK���ڻ�ӦACK */
	IICSTAT = (1<<4);
		
	/* 2. �Ѵ��豸��ַд��IICDS */
	IICDS = (msg->addr<<1)|(1<<0);
	
	/* 3. IICSTAT = 0xb0 , ���豸��ַ�������ͳ�ȥ, �������жϲ��� */
	IICSTAT = 0xb0;
	

	/* �����Ĵ������ж����� */

	/* ѭ���ȴ��жϴ������ */
	while (!msg->err && msg->cnt_transferred != msg->len);

	if (msg->err)
		return -1;
	else 
		return 0;
}

int s3c2440_master_xfer(p_i2c_msg msgs, int num)
{
	int i;
	int err;
	for(i = 0;i < num;i++)
	{
		if(msgs[i].flags == 0)/*write*/
			err = do_master_tx(&msgs[i]);
		else
			err = do_master_rx(&msgs[i]);
		if(err)
			return err;
	}
	return 0;//ȫ��������
	
}

/* ÿ��оƬʵ���Լ���i2c_controller
          .init
          .master_xfer
          .name
 */
 static i2c_controller s3c2440_i2c_con = {
	.name = "s3c2440",
	.init = s3c2440_i2c_con_init,
	.master_xfer = s3c2440_master_xfer,
 };
void s3c2440_i2c_con_add(void)
{
	register_i2c_controller(&s3c2440_i2c_con);//����̬ȫ�ֽṹ��ָ������ĵ�ַ �ŵ��ṹ��ָ��������
}

