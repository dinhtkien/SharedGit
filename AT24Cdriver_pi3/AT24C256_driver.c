#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/random.h>

static struct class *eep_class = NULL;
static int probed_ndevices = 0;
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);
struct eep_dev {
    unsigned char *data;
    struct i2c_client *client;
    struct mutex eep_mutex;
    struct list_head    device_entry;
    dev_t       devt;
    unsigned    users;
};





static bool AT24Cwriteable_reg(struct device *dev, unsigned int reg)
{
    if(reg >= 0 && reg <= (u16)0x7FFF)
    {
        return true;
    }
    return false;
};
static bool AT24Creadable_reg(struct device *dev, unsigned int reg)
{
    if(reg >= 0 && reg <= (u16)0x7FFF)
    {
        return true;
    }
    return false;
};
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
static struct regmap *AT24regmap;
static const struct regmap_config AT24Cregmap_config = 
{
    .reg_bits = 16,
    .val_bits = 8,
    .max_register = (u16)0x7FFF,
    .fast_io = false,
    .writeable_reg = AT24Cwriteable_reg,
    .readable_reg = AT24Creadable_reg,
};
// static void data_dump(u8 *data, u32 count)
// {
//     pr_info("AT24C256 driver: start dumping\n");
//     for (u32 index = 0; index < count; index++)
//     {
//         pr_info("AT24C256 driver:%x", *(data+index));
//     }
//     pr_info("AT24C256 driver: done dumping\n");
// }
static int AT24C256_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
    struct device_node *AT24C256_device_node_p = client->dev.of_node;
    u32 i2c_data = 0;
    struct device *device = NULL;

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

    AT24regmap = devm_regmap_init_i2c(client, &AT24Cregmap_config);
    if (IS_ERR(AT24regmap))
    {
        pr_err("Cannot init register mapping to i2c device");
        return -EFAULT;
    };
    //perform write/read in 0x7fff to test eeprom
    i2c_data = (u32)get_random_u8();
    regmap_write(AT24regmap, (u16)0x7fff, (u8)i2c_data);
    pr_info("AT24C256 driver: i2c data to write %d\n", i2c_data);
    mdelay(5);
    regmap_read(AT24regmap, (u16)(0x7fff), &i2c_data);
    mdelay(5);
    pr_info("AT24C256 driver: read i2c data %d\n", i2c_data);

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
    u8 ret_value = 0;
    eep_class = class_create(THIS_MODULE, "eeprom");
     if (IS_ERR(eep_class)) 
     {
        pr_err("AT24C256 driver: error creating eep class");
        ret_value = PTR_ERR(eep_class);
        goto EEP_CLASS_CREATE_FAILED;
     };

    if (ret_value = i2c_register_driver(THIS_MODULE, &AT24C256_i2c_driver) < 0) 
    {
        pr_err("AT24C256 driver: error registering AT24C256_i2c_driver, error code:%d exitting\n", ret_value);
        goto I2C_DRIVER_REGISTRATION_FAILED;
    };
    pr_info("AT24C256 driver: AT24C256_i2c_driver registered\n");
    goto RETURN_LABEL;
I2C_DRIVER_REGISTRATION_FAILED:
    class_destroy(eep_class);
EEP_CLASS_CREATE_FAILED:
    //nothing to do
RETURN_LABEL:
    return ret_value;
}
static void __exit AT24C256_driver_exit (void)
{
    i2c_del_driver(&AT24C256_i2c_driver);
    class_destroy(eep_class);
    pr_info("AT24C256 driver: AT24C256_i2c_driver unregistered\n");
}
module_init(AT24C256_driver_init);
module_exit(AT24C256_driver_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");