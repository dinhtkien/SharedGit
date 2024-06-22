#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/gpio.h>
#include <linux/delay.h>
static int ledGPIO = 570;
module_param(ledGPIO, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
MODULE_PARM_DESC(ledGPIO,"this is ledGPIO");
MODULE_INFO(LEDGPIODriver,"ledgpio driver for fun");

static int __init ledxxxparamdriver_init(void)
{	
	gpio_request(ledGPIO, "sysfs");
	gpio_direction_output(ledGPIO, 1);
	gpio_set_value(ledGPIO, 1);
	printk(KERN_INFO "GPIO %d state:%d\n", ledGPIO, gpio_get_value(ledGPIO));
	printk(KERN_INFO "LED TESTING\n");
	return 0;
}
static void __exit ledxxxparamdriver_exit(void)
{
	printk("Goodbye from led driver");
	gpio_set_value(ledGPIO, 0);
	gpio_free(ledGPIO);
	return;
}

module_init(ledxxxparamdriver_init);
module_exit(ledxxxparamdriver_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");
