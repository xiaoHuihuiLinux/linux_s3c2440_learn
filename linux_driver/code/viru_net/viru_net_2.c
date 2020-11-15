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

static struct net_device *vnet_dev; //只在本模块用的话最好加上static

static int virt_net_sendpacket(struct sk_buff *skb, struct net_device *dev)//这是硬件相关的操作才会到这里假如我门设置ip为3.3.3.3，去ping别的ip会报错
{
	static unsigned int  Tx_cnt =0;
	printk("the tx pack is %d\n",++Tx_cnt);
	//更新统计信息
	dev->stats.tx_packets++;//发地包数
	dev->stats.tx_bytes += skb->len;//字节数
	//同样也可以设置默认mac地址
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
	/*网卡驱动：
	*分配net_device结构体
	*设置
	*注册
	*/
	//viru_dev = alloc_etherdev(sizeof(struct net_local));//这个函数也是调用alloc_netdev我们不用eth之类的名字
	//分配
	vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);//第一个参数私有数据设置为0
	//设置发包函数
	vnet_dev->hard_start_xmit = virt_net_sendpacket;//vnet_dev指针的话最->符号
	//注册
	//register_netdevice(viru_dev);//记得我们是调用register_netdev不是register_netdevice
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


