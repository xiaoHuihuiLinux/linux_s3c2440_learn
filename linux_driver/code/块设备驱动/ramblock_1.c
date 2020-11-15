#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>

#define RAMBLOCK_SIZE (1024*1024)

static struct gendisk *ramblock_disk;
static request_queue_t *ramblock_queue;

static DEFINE_SPINLOCK(ramblock_lock);

static void do_ramblock_request (request_queue_t * q)//处理”队列中的请求的函数
{
	static int cnt = 0;
	struct request *req;
	printk("do_rambloc_request", ++cnt);
	/*
	从电梯调度算法”从“request_queue_t * q”队列里面取出下一个请求“req =
	elv_next_request(q)”，然后执行这个请求“req
	*/
   //当我们加载驱动后，马上用到了“do_ramblock_request”队列请求处理函数。但到这里后就再也
   //不能返回到shell 了。因为这个函数对“队列 equest_queue_t * q”没有任何处理。所以
   //没有再返回。所以我们就要就要加上下面的while 
	while ((req = elv_next_request(q)) != NULL) {
			end_request(req, 1);
	}
}
static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
};

static int major;
static int ramblock_init(void)
{
	/*1.分配gendisk结构体
	* 2.设置
	* 2.1分配一个队列request_queue_t提供读写能力request_queue_t
	* 2.2设置gendisk的属性
	* 3.注册gendisk 
	*/
	//1分配gendisk结构体
	/*为1 时，就是把整个磁盘当成一个分区，则在上面不能再创建分
	区了。如写成16，则最多可以创建15 个分区
	*/
	ramblock_disk = alloc_disk(16);//参数指次设备号个数即“分区个数+0”。0 是指整个磁盘
	//2.分配/设置队列 提供读写能力
	/*我们在分析ll_rw_block的时候，1.首先用buffer_head 来构造bio，2.然后设置bio
	3.然后提交submit_bio()4.把bio放入构造请求__generic_make_request(bio)5.然后调用make_request_fn函数（ q->make_request_fn(q, bio)）
	6.在函数blk_init_queue_node（）中调用blk_queue_make_request（）继而设置make_request_fn他会提供一个
	默认构造请求的函数“__make_request。blk_init_queue_node（）的参数1就是我们自己的请求处理函数
	*/
	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);//参数1执行处理队列的函数，参数2一个自旋锁
	ramblock_disk->queue = ramblock_queue;//设置队列
	//2.2设置属性
	/*主设备号可以自已定义，也可以让系统自动分配。Register_blkdev()函数退化了，
	*用此函数当参1 为“0”时可以让系统自动分配一个主设备
	*/
	major = register_blkdev(0, "ramblock");//自动分配主设备号
	ramblock_disk->major = major;//
	ramblock_disk->first_minor = 0;//第一个次设备号写为0，则从0~16都对应这个块设备
	sprintf(ramblock_disk->disk_name, "ramblock");//块设备的名字
	ramblock_disk->fops = &ramblock_fops;
	//ramblock_disk->private_data = p;
	set_capacity(ramblock_disk, RAMBLOCK_SIZE/512);//设置容量 单位是“扇区”，在内核里，对于文件系统那一层，永远认为扇区是512 字节。
	//3.注册
	add_disk(ramblock_disk);
	return 0;
}
static void ramblock_exit(void)//可以参考Z2ram.c
{
	unregister_blkdev(major,"ramblock"); //卸载块设备
	del_gendisk(ramblock_disk); //清除gendisk结构
	put_disk(ramblock_disk);//释放块设备结构空间
	blk_cleanup_queue (ramblock_queue); //清除队列.
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");



