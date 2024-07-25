#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/delay.h>
static  unsigned int led = 518;
static int led23driver_init(void)
{
    gpio_request(led,"sysfs");
    gpio_direction_output(led, 1);
    gpio_set_value(led, 1);
    for (int a = 0; a<= 100; a++)
    {
	gpio_set_value(led, 1);
	msleep(1000);
	gpio_set_value(led, 0);
	msleep(1000);
	pr_info("%d",a);
    }
    pr_info("led23driver init successfully");
    return 0;
}
static void led23driver_exit(void)
{
    gpio_set_value(led, 0);
    gpio_free(led);
    pr_info("led23driver exit successfully");
    return;
}
module_init(led23driver_init);
module_exit(led23driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("dinhtkien");
MODULE_DESCRIPTION("A led test driver for the RPi");
MODULE_VERSION("0.1");
