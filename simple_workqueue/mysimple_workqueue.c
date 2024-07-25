#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>

static struct my_data_struct
{
	struct work_struct my_workqueue;
	int myint;
};
static struct my_data_struct *my_data;

static void work_handler(struct work_struct *work)
{
	struct my_data_struct *data = container_of(work, struct my_data_struct, my_workqueue);
	pr_info("before sleepin work handler\n");
	msleep(5000);
	pr_info("after sleep in  work handler, data = %d\n", data->myint);
	kfree(my_data);
}


static int __init my_init_func(void)
{
	my_data = kmalloc(sizeof(struct my_data_struct),GFP_KERNEL);
	my_data->myint = 1233;
	INIT_WORK(&(my_data->my_workqueue),work_handler);
	queue_work(system_wq, &(my_data->my_workqueue));
	pr_info("in init function\n");
	return 0;
}
static void __exit my_exit_func(void)
{
	//kfree(my_data);
	pr_info("exitting\n");
}


module_init(my_init_func);
module_exit(my_exit_func);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");

