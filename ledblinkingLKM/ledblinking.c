#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("dinhtkien");
MODULE_DESCRIPTION("A Button/LED test driver for the RPi");
MODULE_VERSION("0.1");

static unsigned int gpioLED = 575;
static int __init erpi_gpio_init(void)
{
	gpio_request(gpioLED, "sysfs");
	gpio_direction_output(gpioLED, 1);
	gpio_set_value(gpioLED, 1);
	printk(KERN_INFO "GPIO 575 state:%d\n", gpio_get_value(gpioLED));
	printk(KERN_INFO "LED TESTING\n");
	return 0;
}
static void __exit erpi_gpio_exit(void)
{
	printk("Goodbye from led driver");
	gpio_set_value(gpioLED, 0);
}
module_init(erpi_gpio_init);
module_exit(erpi_gpio_exit);
