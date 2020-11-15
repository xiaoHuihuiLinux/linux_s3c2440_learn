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
struct net_device *vnet_dev; 

static int viru_net_init(void)
{
	/*网卡驱动：
	*分配net_device结构体
	*设置
	*注册
	*/
	//viru_dev = alloc_etherdev(sizeof(struct net_local));//这个函数也是调用alloc_netdev我们不用eth之类的名字
	//分配
	vnet_dev = alloc_netdev(0, "vent%d", ether_setup);//第一个参数私有数据设置为0
	//设置暂且不做
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


