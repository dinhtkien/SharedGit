#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/delay.h>
// static struct class *eep_class = NULL;
static struct AT24C256_node
{
    char *name;
    u8 addr_cell_num;
    u8 size_cell_num;
    char *compatible;
    u16 reg;
    char *status;
    u16 bytesize;
};
/* Perform read wrapper to eeprom device
    params: *client: eeprom device need to be read from
            read_address: starting register address  need to be read from
            *read_buff: output buffer
            count: number of byte need to be read
*/
static int eeprom_read(struct i2c_client *client, u16 read_address, u8 *read_buff, u16 count, struct AT24C256_node AT24C256_nd)
{
    u8 *i2c_rdata;
    struct i2c_msg i2c_rmessage[2];
    
    if (read_address > AT24C256_nd.bytesize)
    {
        pr_err("AT24C256 driver: bad read_address");
        return -EFAULT;
    }

    i2c_rdata = kmalloc(count + 2, GFP_KERNEL);
    i2c_rdata[0] = (u8)(read_address >> 8);
    i2c_rdata[1] = (u8)(read_address && 0xff);

    i2c_rmessage[0].addr = client->addr;
    i2c_rmessage[0].flags = client->flags;         /* Write */
    i2c_rmessage[0].len = 2;                      /* Address is 2byte coded */
    i2c_rmessage[0].buf = i2c_rdata;

    i2c_rmessage[1].addr = client->addr;
    i2c_rmessage[1].flags = client->flags | I2C_M_RD;             /* We need to read */
    i2c_rmessage[1].len = 16; 
    i2c_rmessage[1].buf = (i2c_rdata + 2);

    if (i2c_transfer(client-> adapter, i2c_rmessage, 2) < 0)
    {
        pr_err("AT24C256 driver: i2c transfer failed");
        return -EIO;
    }
    read_buff = (i2c_rdata + 2);
}
static int AT24C256_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
    struct device_node *AT24C256_device_node_p = client->dev.of_node;
    u8 i2c_rdata[18];
    struct i2c_msg i2c_rmessage[2];
    int return_value = 0;
    struct AT24C256_node AT24C256_nd = 
    {
        .name = (char*)of_node_full_name(AT24C256_device_node_p),
        .addr_cell_num = of_n_addr_cells(AT24C256_device_node_p),
        .size_cell_num = of_n_size_cells(AT24C256_device_node_p),
        .compatible = (char *)of_get_property(AT24C256_device_node_p, "compatible", NULL),
        .reg = (unsigned long)of_read_number(of_get_property(AT24C256_device_node_p,"reg",NULL), of_n_addr_cells(AT24C256_device_node_p)),
        .status = (char *)of_get_property(AT24C256_device_node_p, "status", NULL),
        .bytesize = (unsigned long)of_read_number(of_get_property(AT24C256_device_node_p,"bytesize",NULL), 1)
    };

    pr_info("AT24C256 driver: node name: %s\n, node status: %s\n, node reg: 0x%x\n, node compatible: %s\n, size: %d", AT24C256_nd.name,\
                                                                                                            AT24C256_nd.status,\
                                                                                                            AT24C256_nd.reg,\
                                                                                                            AT24C256_nd.compatible,\
                                                                                                            AT24C256_nd.bytesize);
    pr_info("AT24C256 driver: client address 0x%x\n client flag: 0x%x\n", client->addr, client->flags);

    if (strncmp(AT24C256_nd.status, "okay", sizeof("okay")))
    {
        pr_info("AT24C256 driver: Device status is disabled. Exitting\n");
        return 0;
    }
    pr_info("AT24C256 driver: executing device driver matching\n");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))  return -EIO;
    /*simple read data with start reg is 0x00, length =16 to test eeprom*/
    i2c_rdata[0] = (u8)0x00;
    i2c_rdata[1] = (u8)0x00;
    i2c_rmessage[0].addr = client->addr;
    i2c_rmessage[0].flags = client->flags;         /* Write */
    i2c_rmessage[0].len = 2;                      /* Address is 2byte coded */
    i2c_rmessage[0].buf = i2c_rdata;          

    i2c_rmessage[1].addr = client->addr;
    i2c_rmessage[1].flags = client->flags | I2C_M_RD;             /* We need to read */
    i2c_rmessage[1].len = 16; 
    i2c_rmessage[1].buf = (i2c_rdata + 2);

    return_value = i2c_transfer(client-> adapter, i2c_rmessage, 2);

    if (return_value < 0)
    {
        pr_err("AT24C256 driver: Error occured when transmit i2c message, error code: %d", return_value);
        return return_value;
    }
    pr_info("AT24C256 driver:transmitted, number of transmitted bytes: %d,i2c data in reg 0x0000 is: %s", return_value, (i2c_rdata + 2));
    pr_info("AT24C256 driver: probed\n");
    return 0;
}


static const struct of_device_id AT24C256EEP_dt_ids[] = {
    { .compatible = "AT24C256",},
    {},
};
MODULE_DEVICE_TABLE(of, AT24C256EEP_dt_ids);

static struct i2c_driver AT24C256_i2c_driver = 
{
    .driver = 
    {
        .owner = THIS_MODULE,
        .name = "AT24C256eeprom",
        .of_match_table = AT24C256EEP_dt_ids,
    },
    .probe = AT24C256_probe,
};

static int __init AT24C256_driver_init (void)
{
    int status;
    /* Claim our 256 reserved device numbers.  Then register a class
     * that will key udev/mdev to add/remove /dev nodes.  Last, register
     * the driver which manages those device numbers.
     */
    // eep_class = class_create(THIS_MODULE, "eeprom");
    // if (IS_ERR(eep_class))
    //     pr_err("error creating eeprom class, exitting\n");
    //     return PTR_ERR(eep_class);

    status = i2c_register_driver(THIS_MODULE, &AT24C256_i2c_driver);
    pr_info("AT24C256 driver: AT24C256_i2c_driver registered\n");
    if (status < 0)
        // class_destroy(eep_class);
        pr_err("AT24C256 driver: error registering AT24C256_i2c_driver, exitting\n");
        return status;
    return 0;
}
static void __exit AT24C256_driver_exit (void)
{
    i2c_del_driver(&AT24C256_i2c_driver);
    // class_destroy(eep_class);
    pr_info("AT24C256 driver: AT24C256_i2c_driver unregistered\n");
}
module_init(AT24C256_driver_init);
module_exit(AT24C256_driver_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");