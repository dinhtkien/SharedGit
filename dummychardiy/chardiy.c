#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

static struct class *dummy_class;
static struct cdev dummy_cdev;
struct device *dummy_device;
dev_t devt = 0;

int dummy_open(struct inode * inode, struct file * filp)
{
    pr_info("Someone tried to open me\n");
    return 0;
}

int dummy_release(struct inode * inode, struct file * filp)
{
    pr_info("Someone closed me\n");
    return 0;
}

ssize_t dummy_read (struct file *filp, char __user * buf, size_t count,
                                loff_t * offset)
{
    pr_info("Reading\n");
    char *data_to_read = "Hi this is dummy char driver\n";
    count = strlen(data_to_read);
    if (*offset >= count)
	{
		pr_info("read end of file. Reset offset");
		*offset = 0;
		return 0;
	}
    pr_info("count data:%ld, offset: %lld\n", count, *offset);
    int error = copy_to_user(buf, data_to_read, count);
    if (error < 0)
    {
	pr_err("error when reading data");
	return -ENOMEM;
    }
    *offset += count;
    return count;
}


ssize_t dummy_write(struct file * filp, const char __user * buf, size_t count,
                                loff_t * offset)
{
    char *data_to_write = kmalloc(count, GFP_KERNEL);
    int error = copy_from_user(data_to_write,buf,count);
    if (error < 0)
    {
	pr_err("cannot allocate kernel memory");
	return -ENOMEM;
    }
    pr_info("Writing\n");
    pr_info("Data to write:%s\n",data_to_write);
    return count;
}

static struct file_operations dummy_fops = {
    open:       dummy_open,
    release:    dummy_release,
    read:       dummy_read,
    write:      dummy_write,
};

static int __init my_init(void)
{
	int error = alloc_chrdev_region(&devt,0,1,"dummy_char");//dynamic allocate major and minor number
	if (error < 0)
	{
		pr_err("Error locating major & minor number\n");
		return error;
	}
	pr_info("Major number is %d and minor number is %d\n", MAJOR(devt), MINOR(devt));
	dummy_class = class_create("dummy_char_class");//create class 
	if (IS_ERR(dummy_class))
	{
		pr_err("error creating dummy class\n");
		return PTR_ERR(dummy_class);
	}
	cdev_init(&dummy_cdev, &dummy_fops);
	dummy_cdev.owner = THIS_MODULE;
	cdev_add(&dummy_cdev, devt, 1);
	dummy_device = device_create(dummy_class,
                                NULL,   /* no parent device */
                                devt,   /* associated dev_t */
                                NULL,   /* no additional data */
                                "dummy_char_device"); /* device name */
	if (IS_ERR(dummy_device))
	{
        	pr_err("Error creating dummy char device.\n");
	        unregister_chrdev_region(devt, 1);
		device_destroy(dummy_class,devt);
		cdev_del(&dummy_cdev);
		class_destroy(dummy_class);
        	return PTR_ERR(dummy_device);
    	}

	pr_info("module loaded successfully\n");
	return 0;
}
static void __exit my_exit(void)
{
	unregister_chrdev_region(devt, 1);
	device_destroy(dummy_class, devt);
	cdev_del(&dummy_cdev);
	class_destroy(dummy_class);
	pr_info("clear successfully, exitting \n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");
