#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
static  unsigned int led = 535;
static int led23driver_init(void)
{
	int return_value = 0;
        gpio_request(led,"sysfs");
        gpio_direction_output(led, 1);
        gpio_set_value(led, 1);
        printk("led23driver insert successfully");
        return return_value;
}
static void led23driver_exit(void)
{
        gpio_set_value(led, 0);
        printk("led23driver remove OK");
}
module_init(led23driver_init);
module_exit(led23driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("dinhtkien");
MODULE_DESCRIPTION("A led test driver for the RPi");
MODULE_VERSION("0.1");


