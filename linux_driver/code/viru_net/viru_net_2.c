#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

static struct net_device *vnet_dev; //ֻ�ڱ�ģ���õĻ���ü���static

static int virt_net_sendpacket(struct sk_buff *skb, struct net_device *dev)//����Ӳ����صĲ����Żᵽ���������������ipΪ3.3.3.3��ȥping���ip�ᱨ��
{
	static unsigned int  Tx_cnt =0;
	printk("the tx pack is %d\n",++Tx_cnt);
	//����ͳ����Ϣ
	dev->stats.tx_packets++;//���ذ���
	dev->stats.tx_bytes += skb->len;//�ֽ���
	//ͬ��Ҳ��������Ĭ��mac��ַ
	dev->dev_addr[0] = 0x08;
    dev->dev_addr[1] = 0x89;
    dev->dev_addr[2] = 0x89;
    dev->dev_addr[3] = 0x89;
    dev->dev_addr[4] = 0x89;
    dev->dev_addr[5] = 0x00;
	return 0;
}
static int viru_net_init(void)
{
	/*����������
	*����net_device�ṹ��
	*����
	*ע��
	*/
	//viru_dev = alloc_etherdev(sizeof(struct net_local));//�������Ҳ�ǵ���alloc_netdev���ǲ���eth֮�������
	//����
	vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);//��һ������˽����������Ϊ0
	//���÷�������
	vnet_dev->hard_start_xmit = virt_net_sendpacket;//vnet_devָ��Ļ���->����
	//ע��
	//register_netdevice(viru_dev);//�ǵ������ǵ���register_netdev����register_netdevice
	register_netdev(vnet_dev);
	return 0;
}
void viru_net_exit(void)
{
	unregister_netdev(vnet_dev);
	free_netdev(vnet_dev);
}
module_init(viru_net_init);
module_exit(viru_net_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jiangHuihui");


