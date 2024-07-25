#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/delay.h>

static struct mutex my_mutex;
static int __init mutex_init_function(void)
{
	mutex_init(&my_mutex);
	mutex_lock(&my_mutex);
	pr_info("Entering mutext\n");
	msleep(3000);
	pr_info("code running in mutex context\n");
	mutex_unlock(&my_mutex);
	return 0;
}
static void __exit mutex_exit_function(void)
{
	pr_info("mutex doesn't need to free. exitting\n");
}

module_init(mutex_init_function);
module_exit(mutex_exit_function);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");
