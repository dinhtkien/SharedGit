#include <linux/init.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
static struct hrtimer myhrtimer;
static int hrtimer_call_count = 0;
static enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	hrtimer_call_count++;
	pr_info("in timer_callback count: %d\n", hrtimer_call_count);
	hrtimer_forward_now(&myhrtimer, ns_to_ktime(300000));
	return HRTIMER_RESTART;
}
static int __init myhrtimer_init(void)
{
	hrtimer_init(&myhrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	myhrtimer.function =  &timer_callback;
	//hrtimer_setup(&myhrtimer, init_time, &timer_callback);
	hrtimer_start(&myhrtimer, ms_to_ktime(1), HRTIMER_MODE_REL);
	pr_info("in init function\n");
return 0;
}
static void __exit myhrtimer_exit(void)
{
	hrtimer_cancel(&myhrtimer);
	pr_info("exitting\n");
}
module_init(myhrtimer_init);
module_exit(myhrtimer_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");
