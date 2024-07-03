#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/delay.h> // for msleep
#include <linux/slab.h>

// Define a work structure to hold data for the work handler
static struct my_work_data {
  int data;
  struct delayed_work my_workqueue;
};
static struct my_work_data *my_work_data_obj;
static void my_work_handler(struct work_struct *work) {
 // struct my_work_data *data = container_of(work, struct my_work_data, my_workqueue);

  // Simulate some work being done by the delayed work
  pr_info("Delayed work triggered! Data: \n");
  kfree(my_work_data_obj);
}


static int __init my_module_init(void) {
  //struct my_work_data *my_work_data_obj;
  my_work_data_obj = kmalloc(sizeof(struct my_work_data), GFP_KERNEL);
  INIT_DELAYED_WORK(&(my_work_data_obj->my_workqueue), my_work_handler);
  my_work_data_obj->data = 42; // Example data

  // Schedule the delayed work to run in 5 seconds
  queue_delayed_work(system_wq, &(my_work_data_obj->my_workqueue), msecs_to_jiffies(5000));

  return 0;
}

static void __exit my_module_exit(void) {
 kfree (my_work_data_obj);
 pr_info("exitting");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dinhtkien");
MODULE_DESCRIPTION("Simple delayed workqueue example");
