#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/init.h>
void tasklet_function(struct tasklet_struct *tasklet)
{
	pr_info("running in callback function %s\n",__func__);
}

static DECLARE_TASKLET(my_tasklet, tasklet_function);
static int __init my_init(void)
{
	/* Schedule the handler */
	tasklet_schedule(&my_tasklet);
	pr_info("tasklet example\n");
	return 0;
}
static void __exit my_exit( void )
{
	/* Stop the tasklet before we exit */
	tasklet_kill(&my_tasklet);
	pr_info("tasklet example cleanup\n");
	return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");
