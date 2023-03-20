#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h> 


#define         NEWCHRLED_MAJOR               (200)
#define         NEWCHRLED_NAME                "newchrled"
#define         NEWCHRLED_COUT                 (1)
/* 寄存器物理地址 */
#define CCM_CCGR1_BASE              		(0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE      		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE      		(0X020E02F4)
#define GPIO1_DR_BASE               		(0X0209C000)
#define GPIO1_GDIR_BASE             		(0X0209C004)

/* 地址映射后的虚拟地址指针 */
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;



#define     LEDOFF          0               /* led关闭 */
#define     LEDON           1               /* led打开 */

/* LED设备结构体 */
typedef struct newchrled_dev{
    struct cdev cdev;   /* 字符设备 */
    dev_t   devid;      /* 设备号 */
    int     major;      /*主设备号 */
    int     minor;      /* 次设备号 */
}newchrled_dev_t;

newchrled_dev_t newchrled;      /* led设备 */

/* LED灯打开/关闭 */
static void led_switch(u8 state)
{
    u32 val;
    if (state == LEDON)
    {
        /* 灯打开  */
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);            /* bit3置0,打开LED灯 */
        writel(val, GPIO1_DR);        
    }
    else
    {
        /* 灯关闭 */
        val = readl(GPIO1_DR);
        val |= (1 << 3);            /* bit3置1关闭LED灯 */
        writel(val, GPIO1_DR);        
    }

    return ;
}


static int newchrled_open(struct inode *inode, struct file *file)
{
    return 0;
}


static ssize_t newchrled_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    int ret = 0;
    unsigned char data_buf[1] = {0};

    ret = copy_from_user(data_buf, buf, len);
    if (ret < 0)
    {
        printk("kernel write failed\r\n");
        return -EFAULT; 
    }

    led_switch(data_buf[0]);    //判断灯开关
    if (data_buf[0])
    {
        printk("LED ON!\r\n");
    }
    else
    {
        printk("LED OFF!\r\n");
    }
    
    



    return 0;
}


static int newchrled_release(struct inode *inode, struct file *file)
{
    return 0;
}


/* 字符设备操作集 */
static const struct file_operations newchrled_fops = {
	.owner	= THIS_MODULE,
	.write	= newchrled_write,
	.open	= newchrled_open,
	.release= newchrled_release,
};


/* 入口*/
static int __init newchrled_init(void)
{
	int ret = 0;
	unsigned int val = 0;
  
	/* 地址映射 */
	IMX6U_CCM_CCGR1			= ioremap(CCM_CCGR1_BASE, 4);
	SW_MUX_GPIO1_IO03		= ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
	SW_PAD_GPIO1_IO03		= ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
	GPIO1_DR				= ioremap(GPIO1_DR_BASE, 4);
	GPIO1_GDIR				= ioremap(GPIO1_GDIR_BASE, 4);


	/* 初始化寄存器 */
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3 << 26);  /* 先清除以前的配置bit26,27 */
    val |= 3 << 26;     /* bit26,27置1 */
    writel(val, IMX6U_CCM_CCGR1);
 
    writel(0x5, SW_MUX_GPIO1_IO03);     /* 设置复用 */
    writel(0X10B0, SW_PAD_GPIO1_IO03);  /* 设置电气属性 */

    val = readl(GPIO1_GDIR);
    val |= 1 << 3;              /* bit3置1,设置为输出 */
    writel(val, GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val |= (1 << 3);            /* bit3置1,关闭LED灯 */
    writel(val, GPIO1_DR);

	//2.注册字符设备
    if (newchrled.major)    //如果给定主设备号
    {
        newchrled.devid = MKDEV(newchrled.major, 0);
        ret = register_chrdev_region(newchrled.devid, NEWCHRLED_COUT, NEWCHRLED_NAME); 
    }
    else
    {
        ret = alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_COUT, NEWCHRLED_NAME);
        newchrled.major = MAJOR(newchrled.devid);
        newchrled.minor = MINOR(newchrled.devid);
    }
    if (ret < 0)
    {
        printk("newchrled chrdev_region err");
        return -1;
    }
    printk("newchrled major=%d, minor = %d\r\n", newchrled.major, newchrled.minor);

    /* 3.注册字符设备 */
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);
    ret = cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_COUT);


	printk("newchrled_init\r\n");

    return 0;
}

/* 出口*/
static void __exit newchrled_exit(void)
{
	unsigned int val = 0;
    printk("newchrled_exit\r\n");

    val = readl(GPIO1_DR);
    val |= (1 << 3);            /* bit3置1,关闭LED灯 */
    writel(val, GPIO1_DR);

	/* 取消地址映射 */
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    /* 删除字符设备 */
    cdev_del(&newchrled.cdev);

	//卸载字符设备
	unregister_chrdev_region(newchrled.devid, NEWCHRLED_COUT);
	
    return ;
}


/* 注册驱动加载和卸载 */
module_init(newchrled_init);
module_exit(newchrled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lizh");






