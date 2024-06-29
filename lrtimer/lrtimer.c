#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/kernel.h>

static struct timer_list my_timer;

static void my_timer_callback( struct timer_list *lrtimer)
{
	pr_info("in timer callback\n");
}

static int __init my_timer_init(void)
{
	timer_setup(&my_timer, my_timer_callback, 0);
	pr_info("Setup timer to fire in 500ms (%ld)\n", jiffies);
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(500));
	return 0;
}
static void  __exit my_timer_exit(void)
{
	del_timer(&my_timer);
	pr_info("exitting\n");
}
module_init(my_timer_init);
module_exit(my_timer_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_DESCRIPTION("Standard timer example");
MODULE_LICENSE("GPL");
