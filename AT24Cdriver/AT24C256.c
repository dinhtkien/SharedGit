#include <linux/init.h>
#include <linux/module.h>

static int __init driver_init (void)
{
    return 0;
}
static void __exit driver_exit (void)
{

}
module_init(driver_init);
module_exit(driver_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");