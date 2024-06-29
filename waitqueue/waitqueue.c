#include <linux/init.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
static int condition = 0;
static struct work_struct mywork;
static struct wait_queue_head mywq;

static void work_handler(struct work_struct *work)
{
	pr_info("entering work\n");
	msleep(5000);
	condition = 1;
	wake_up(&mywq);
}
static int __init waitqueue_init_function(void)
{
	INIT_WORK(&mywork, work_handler);
	schedule_work(&mywork);

	init_waitqueue_head(&mywq);
	pr_info("entering wait event\n");
	wait_event(mywq, condition != 0);
	pr_info("waking up\n");
	return 0;
}
static void __exit waitqueue_exit_function(void)
{
	pr_info("exiting\n");
}

module_init(waitqueue_init_function);
module_exit(waitqueue_exit_function);

MODULE_AUTHOR("dinhtkien");
MODULE_DESCRIPTION("example use of wait queue");
MODULE_LICENSE("GPL");
