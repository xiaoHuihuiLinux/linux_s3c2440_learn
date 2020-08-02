#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define DEVICE_NAME     "leds"  /* 加载模式后，执行”cat /proc/devices”命令看到的设备名称 */
#define LED_MAJOR       231     /* 主设备号 */


static struct class *leds_class;
static struct class_device	*leds_class_devs[4];


/* bit0<=>D10, 0:亮, 1:灭 
 *  bit1<=>D11, 0:亮, 1:灭 
 *  bit2<=>D12, 0:亮, 1:灭 
 */ 
static char leds_status = 0x0; //为什莫要给这个变量加上信号量也就是互斥锁以为 不仅要read操作还要wrtie操作，在open 的时候清零
//DECLARE_MUTEX(name) 该宏声明一个信号量name并初始化他的值为1，即声明一个互斥锁
static DECLARE_MUTEX(leds_lock); // 定义赋值　
//static int minor;
//GPFCON 0x56000050      的地址
//GPFDAT 0x56000054      的地址
/*gpio_va在初始化的时候我们映射了物理地址0x56000000*/
static unsigned long gpio_va;
#define GPIO_OFT(x) ((x) - 0x56000000)
#define GPFCON  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000050)))
#define GPFDAT  (*(volatile unsigned long *)(gpio_va + GPIO_OFT(0x56000054)))


/* 应用程序对设备文件/dev/leds执行open(...)时，
 * 就会调用s3c24xx_leds_open函数
 */
static int s3c24xx_leds_open(struct inode *inode, struct file *file)
{
	//取出不同的次设备号
	int minor = MINOR(inode->i_rdev); //MINOR(inode->i_cdev);
	switch(minor)
	{
        case 0: /* /dev/leds 操作所有的led*/
        {
            // 配置3引脚为输出
            //s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
            GPFCON &= ~(0x3<<(4*2));//GPFCON 最后的都地址还是0x56000050
            GPFCON |= (1<<(4*2));
            
            //s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
            GPFCON &= ~(0x3<<(5*2));
            GPFCON |= (1<<(5*2));

            //s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
            GPFCON &= ~(0x3<<(6*2));
            GPFCON |= (1<<(6*2));

            // 都输出0
            //s3c2410_gpio_setpin(S3C2410_GPF4, 0);
            GPFDAT &= ~(1<<4);
            
            //s3c2410_gpio_setpin(S3C2410_GPF5, 0);
            GPFDAT &= ~(1<<5);
            //s3c2410_gpio_setpin(S3C2410_GPF6, 0);
            GPFDAT &= ~(1<<6);
			/*
			void down(struct semaphore * sem); 
			该函数用于获得信号量sem，他会导致睡眠，因此不能在中断上下文（包括IRQ上下文和softirq上下文）使用该函数。
			该函数将把sem的值减1，如果信号量sem的值非负，就直接返回，否则调用者将被挂起，直到别的任务释放该信号量才能继续运行。
			*/
            down(&leds_lock);
            leds_status = 0x0;
            up(&leds_lock);
            /*
            void up(struct semaphore * sem); 
            该函数释放信号量sem，即把sem的值加1，如果sem的值为非正数，
            表明有任务等待该信号量，因此唤醒这些等待者。
			*/  
            break;
        }

        case 1: /* /dev/led1 */
        {
            s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
            s3c2410_gpio_setpin(S3C2410_GPF4, 0);
            
            down(&leds_lock);
            leds_status &= ~(1<<0);
            up(&leds_lock);
            
            break;
        }

        case 2: /* /dev/led2 */
        {
            s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
            s3c2410_gpio_setpin(S3C2410_GPF5, 0);
            leds_status &= ~(1<<1);
            break;
        }
		case 3: /* /dev/led3 */
        {
            s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
            s3c2410_gpio_setpin(S3C2410_GPF6, 0);

            down(&leds_lock);
            leds_status &= ~(1<<2);
            up(&leds_lock);
            break;
        }
   }
	return 0;
}
static int s3c24xx_leds_read(struct file *filp, char __user *buff, 
                                         size_t count, loff_t *offp)
{
	int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
    char val;
	switch (minor)
    {
        case 0: /* /dev/leds */
        {
            
            copy_to_user(buff, (const void *)&leds_status, 1);                    
            break;
        }
		case 1: /* /dev/led1 */
        {
            down(&leds_lock);
            val = leds_status & 0x1;
            up(&leds_lock);
            copy_to_user(buff, (const void *)&val, 1);
            break;
        }
		case 2: /* /dev/led2 */
        {
            down(&leds_lock);
            val = (leds_status>>1) & 0x1;
            up(&leds_lock);
            copy_to_user(buff, (const void *)&val, 1);
            break;
        }
		case 3: /* /dev/led3 */
        {
            down(&leds_lock);
            val = (leds_status>>2) & 0x1;
            up(&leds_lock);
            copy_to_user(buff, (const void *)&val, 1);
            break;
        }
        
    }

    return 1;
}
static ssize_t s3c24xx_leds_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    //int minor = MINOR(inode->i_rdev); //MINOR(inode->i_cdev);
	int minor = MINOR(file->f_dentry->d_inode->i_rdev);
    char val;

    copy_from_user(&val, buf, 1);

    switch (minor)
    {
        case 0: /* /dev/leds */
        {            
            s3c2410_gpio_setpin(S3C2410_GPF4, (val & 0x1));
            s3c2410_gpio_setpin(S3C2410_GPF5, (val & 0x1));
            s3c2410_gpio_setpin(S3C2410_GPF6, (val & 0x1));
            down(&leds_lock);
            leds_status = val;
            up(&leds_lock);
            break;
        }

        case 1: /* /dev/led1 */
        {
            s3c2410_gpio_setpin(S3C2410_GPF4, val);//s3c2410_gpio_setpin系统函数

            if (val == 0)
            {
                down(&leds_lock);
                leds_status &= ~(1<<0);
                up(&leds_lock);
            }
            else
            {
                down(&leds_lock);
                leds_status |= (1<<0);                
                up(&leds_lock);
            }
            break;
        }

        case 2: /* /dev/led2 */
        {
            s3c2410_gpio_setpin(S3C2410_GPF5, val);
            if (val == 0)
            {
                down(&leds_lock);
                leds_status &= ~(1<<1);
                up(&leds_lock);
            }
            else
            {
                down(&leds_lock);
                leds_status |= (1<<1);                
                up(&leds_lock);
            }
            break;
        }
		case 3: /* /dev/led3 */
        {
            s3c2410_gpio_setpin(S3C2410_GPF6, val);
            if (val == 0)
            {
                down(&leds_lock);
                leds_status &= ~(1<<2);
                up(&leds_lock);
            }
            else
            {
                down(&leds_lock);
                leds_status |= (1<<2);                
                up(&leds_lock);
            }
            break;
        }
        
    }

    return 1;
}
/* 这个结构是字符设备驱动程序的核心
 * 当应用程序操作设备文件时所调用的open、read、write等函数，
 * 最终会调用这个结构中指定的对应函数
 */
static struct file_operations s3c24xx_leds_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   s3c24xx_leds_open,     
	.read	=	s3c24xx_leds_read,	   
	.write	=	s3c24xx_leds_write,	   
};
/*
 * 执行insmod命令时就会调用这个函数 
 */
static int __init s3c24xx_leds_init(void)
{
    int ret;
	int minor = 0;
	gpio_va = ioremap(0x56000000, 0x100000);//映射的地址以及映射的字节数
	if (!gpio_va) {
		return -EIO;
	}
	/* 注册字符设备
     * 参数为主设备号、设备名字、file_operations结构；
     * 这样，主设备号就和具体的file_operations结构联系起来了，
     * 操作主设备为LED_MAJOR的设备文件时，就会调用s3c24xx_leds_fops中的相关成员函数
     * LED_MAJOR可以设为0，表示由内核自动分配主设备号
     */
    ret = register_chrdev(LED_MAJOR, DEVICE_NAME, &s3c24xx_leds_fops);
    if (ret < 0) {
      printk(DEVICE_NAME " can't register major number\n");
      return ret;
    }
	leds_class = class_create(THIS_MODULE, "leds");
	if (IS_ERR(leds_class))
		return PTR_ERR(leds_class); 
	//创建一个类。目的是在sys下生成设备信息，mdev会根据设备信息自动创建设备节点
	leds_class_devs[0] = class_device_create(leds_class, NULL, MKDEV(LED_MAJOR, 0), NULL, "leds"); /* /dev/leds */
	for (minor = 1; minor < 4; minor++)  /* /dev/led1,2,3 */
	{
		//加载模块的时候，用户空间中的udev会自动响应device_create(…)函数，去/sysfs下寻找对应的类从而创建设备节点。
		leds_class_devs[minor] = class_device_create(leds_class, NULL, MKDEV(LED_MAJOR, minor), NULL, "led%d", minor);
		if (unlikely(IS_ERR(leds_class_devs[minor])))
			return PTR_ERR(leds_class_devs[minor]);
	}
    printk(DEVICE_NAME " initialized\n");
    return 0;
}
/*
 * 执行rmmod命令时就会调用这个函数 
 */
static void __exit s3c24xx_leds_exit(void)
{
	int minor;
    /* 卸载驱动程序 */
    unregister_chrdev(LED_MAJOR, DEVICE_NAME);

	for (minor = 0; minor < 4; minor++)
	{
		class_device_unregister(leds_class_devs[minor]);
	}
	class_destroy(leds_class);
    iounmap(gpio_va);
}
/* 这两行指定驱动程序的初始化函数和卸载函数 */
module_init(s3c24xx_leds_init);
module_exit(s3c24xx_leds_exit);
/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("http://www.100ask.net");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 LED Driver");
MODULE_LICENSE("GPL");



