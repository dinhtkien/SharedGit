#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/init.h>

static spinlock_t my_spinlockirq;
static unsigned long flags;
int __init spinlock_init(void)
{
	spin_lock_init(&my_spinlockirq);
	spin_lock_irqsave(&my_spinlockirq, flags);
	pr_info("Do in spin lock irqsave");
	spin_unlock_irqrestore(&my_spinlockirq, flags);
	return 0;
}
void __exit spinlock_exit(void)
{
	pr_info("spinlock doesn't need to be free. exitting");
}

module_init(spinlock_init);
module_exit(spinlock_exit);

MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");
