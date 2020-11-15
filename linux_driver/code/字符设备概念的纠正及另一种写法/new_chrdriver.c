#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#define HELLO_CNT 2
int major;
static struct cdev hello_cdev; 
static struct cdev hello2_cdev; 

static struct class*cls;
static int hello_open(struct inode *inode, struct file *file)
{
	printk("hello word\n");
	return 0;
}
static int hello2_open(struct inode *inode, struct file *file)
{
	printk("hello2_open\n");
	return 0;
}
static struct file_operations hello_fops = {
	.owner  = THIS_MODULE,
	.open = hello_open,
}; 
static struct file_operations hello2_fops = {
	.owner = THIS_MODULE,
	.open  = hello2_open,
};
static int  chardev_init(void)
{
	dev_t devid;
	/* 3. 告诉内核 */
#if 0
	major = register_chrdev(0,"hello",&hello_fops);
#else 
	if (major) {
		devid = MKDEV(major, 0);//这样就表示（major, 0-1都对应的char_drv）
		register_chrdev_region(devid, HELLO_CNT, "hello");
	} else {
		alloc_chrdev_region(&devid, 0, HELLO_CNT, "hello");
		major = MAJOR(devid);
	}
	cdev_init(&hello_cdev, &hello_fops);
	cdev_add(&hello_cdev, devid, HELLO_CNT);
	//我们为hello2继续创建一个fileopreation
	devid = MKDEV(major, 2);
	register_chrdev_region(devid, 1, "hello2");
	cdev_init(&hello2_cdev, &hello2_fops);
	cdev_add(&hello2_cdev, devid, 1);
	
#endif

	cls = class_create(THIS_MODULE, "hello");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "hello0"); /* /dev/hello0 */
	class_device_create(cls, NULL, MKDEV(major, 1), NULL, "hello1"); /* /dev/hello1 */
	class_device_create(cls, NULL, MKDEV(major, 2), NULL, "hello2"); /* /dev/hello2 */
	//打开hello3肯定是打不开的
	class_device_create(cls, NULL, MKDEV(major, 3), NULL, "hello3"); /* /dev/hello3 */
	
	
	return 0;
}
static void chardev_exit(void)
{
	class_device_destroy(cls, MKDEV(major, 0));
	class_device_destroy(cls, MKDEV(major, 1));
	class_device_destroy(cls, MKDEV(major, 2));
	class_device_destroy(cls, MKDEV(major, 3));
	class_destroy(cls);
	cdev_del(&hello_cdev);
	unregister_chrdev_region(MKDEV(major, 0), HELLO_CNT);
	cdev_del(&hello2_cdev);
	unregister_chrdev_region(MKDEV(major, 2), 1);
}
module_init(chardev_init);
module_exit(chardev_exit);
MODULE_LICENSE("GPL");

