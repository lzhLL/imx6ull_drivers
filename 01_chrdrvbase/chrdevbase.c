#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#define		CHARDEVBASE_MAJOR			(200)						//主设备号
#define		CHARDEVBASE_NAME			"chrdevbase"				//名字


static int chrdevbase_open(struct inode *inode, struct file *filp)
{
	printk("chrdevbase_open\r\n");

	return 0;
}


static int chrdevbase_release(struct inode *inode, struct file *filp)
{
	printk("chrdevbase_release\r\n");
	return 0;
}


static ssize_t chrdevbase_read(struct file *file, char __user *buffer, size_t count,
			 loff_t *ppos)
{
	printk("chrdevbase_read\r\n");
	return 0;
}


static ssize_t chrdevbase_write(struct file *file, const char __user *buf,
	size_t count, loff_t *off)
{
	printk("chrdevbase_write\r\n");
	return 0;
}


static struct file_operations chrdevbase_fops = {
	.owner 		= THIS_MODULE,
	.open 		= chrdevbase_open,
	.release 	= chrdevbase_release,
	.read 		= chrdevbase_read,
	.write		= chrdevbase_write,
};



/*
	模块入口与出口
 */

// static inline int register_chrdev(unsigned int major, const char *name,
// 				  const struct file_operations *fops)
static int __init chrdevbase_init(void)
{
	int ret = 0;
	printk("chrdevbase_init\r\n");

	ret = register_chrdev(CHARDEVBASE_MAJOR, CHARDEVBASE_NAME, &chrdevbase_fops);	//注册字符设备驱动
	if(ret < 0) {
		printk("chrdevbase init failed!\r\n");
	}
	return 0;
}

static void __exit chrdevbase_exit(void)
{
	printk("chrdevbase_exit\r\n");
	unregister_chrdev(CHARDEVBASE_MAJOR, CHARDEVBASE_NAME);		//卸载字符设备驱动
	return;
}


 module_init(chrdevbase_init);				//入口
 module_exit(chrdevbase_exit);				//出口
 
 MODULE_LICENSE("GPL");					//许可证
 MODULE_AUTHOR("lizh");					//作者
