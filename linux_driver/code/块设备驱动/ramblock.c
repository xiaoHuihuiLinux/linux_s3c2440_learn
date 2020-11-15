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
static unsigned char *ramblock_buffer;
static DEFINE_SPINLOCK(ramblock_lock);
//����һ�����Ӳ�̵Ĳ�����
/*
1.����
2.��ʽ��
3.�ҽ�
4.��д
*/
static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	
	/*���������� = ��ͷ*����*����*512*/
	geo->heads = 2;//��ͷ����
	geo->cylinders = 32;//����ĸ���
	geo->sectors = RAMBLOCK_SIZE/2/32/512;//ÿ������512�ֽ�
	return 0;
}

static void do_ramblock_request (request_queue_t * q)//���������е�����ĺ���
{
	static int r_cnt = 0;
	static int w_cnt = 0;
	struct request *req;//requet�ṹ��������У�Դ Ŀ�� ����
	//printk("do_rambloc_request", ++cnt);
	/*
	�ӵ��ݵ����㷨���ӡ�request_queue_t * q����������ȡ����һ������req =
	elv_next_request(q)����Ȼ��ִ���������req
	*/
   //�����Ǽ��������������õ��ˡ�do_ramblock_request����������������������������Ҳ
   //���ܷ��ص�shell �ˡ���Ϊ��������ԡ����� equest_queue_t * q��û���κδ�������
   //û���ٷ��ء��������Ǿ�Ҫ��Ҫ���������while 
	while ((req = elv_next_request(q)) != NULL) {
	unsigned long offset = req->sector *512;//����512-1��������С.
	unsigned long len  = req->current_nr_sectors * 512;//��ǰҪ������ٸ�����
	//���ݴ������Ҫ�أ�Դ Ŀ�� ����
	if(rq_data_dir(req) == READ)//��д�ķ���
	{//memcpy(Ŀ�ģ�Դ������)
		printk("do_rambloc_request read %d\n", ++r_cnt);
		memcpy(req->buffer,ramblock_buffer + offset,len);//���Ѵ��̵����ݶ���buffer
	}
	else 
	{
		printk("do_rambloc_request write %d\n", ++w_cnt);
		memcpy(ramblock_buffer + offset,req->buffer,len);//д��buffer������д��������
	}
	end_request(req, 1);
	}
}
//Ϊ��ʹ��fdisk����ʽ��������������Ҫ�����ṩһЩ��Ϣ ��ͷ ���� ֮�����Ϣ
//���ձ�Ĳ���
static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
	.getgeo = ramblock_getgeo,	
};

static int major;
static int ramblock_init(void)
{
	/*1.����gendisk�ṹ��
	* 2.����
	* 2.1����һ������request_queue_t�ṩ��д����request_queue_t
	* 2.2����gendisk������
	* 3.ע��gendisk 
	*/
	//1����gendisk�ṹ��
	/*Ϊ1 ʱ�����ǰ��������̵���һ���������������治���ٴ�����
	���ˡ���д��16���������Դ���15 ������
	*/
	ramblock_disk = alloc_disk(16);//����ָ���豸�Ÿ���������������+0����0 ��ָ��������
	//2.����/���ö��� �ṩ��д����
	/*�����ڷ���ll_rw_block��ʱ��1.������buffer_head ������bio��2.Ȼ������bio
	3.Ȼ���ύsubmit_bio()4.��bio���빹������__generic_make_request(bio)5.Ȼ�����make_request_fn������ q->make_request_fn(q, bio)��
	6.�ں���blk_init_queue_node�����е���blk_queue_make_request�����̶�����make_request_fn�����ṩһ��
	Ĭ�Ϲ�������ĺ�����__make_request��blk_init_queue_node�����Ĳ���1���������Լ�����������
	*/
	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);//����1ִ�д�����еĺ���������2һ��������
	ramblock_disk->queue = ramblock_queue;//���ö���
	//2.2��������
	/*���豸�ſ������Ѷ��壬Ҳ������ϵͳ�Զ����䡣Register_blkdev()�����˻��ˣ�
	*�ô˺�������1 Ϊ��0��ʱ������ϵͳ�Զ�����һ�����豸
	*/
	major = register_blkdev(0, "ramblock");//�Զ��������豸��
	ramblock_disk->major = major;//
	ramblock_disk->first_minor = 0;//��һ�����豸��дΪ0�����0~16����Ӧ������豸
	sprintf(ramblock_disk->disk_name, "ramblock");//���豸������
	ramblock_disk->fops = &ramblock_fops;
	//ramblock_disk->private_data = p;
	set_capacity(ramblock_disk, RAMBLOCK_SIZE/512);//�������� ��λ�ǡ������������ں�������ļ�ϵͳ��һ�㣬��Զ��Ϊ������512 �ֽڡ�
	//Ӳ������
	ramblock_buffer = kzalloc(RAMBLOCK_SIZE,GFP_KERNEL);//���ڴ����һ�����豸
	//4.ע��
	add_disk(ramblock_disk);
	return 0;
}
static void ramblock_exit(void)//���Բο�Z2ram.c
{
	unregister_blkdev(major,"ramblock"); //ж�ؿ��豸
	del_gendisk(ramblock_disk); //���gendisk�ṹ
	put_disk(ramblock_disk);//�ͷſ��豸�ṹ�ռ�
	blk_cleanup_queue (ramblock_queue); //�������.
	kfree(ramblock_buffer);
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");



