#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/random.h>
#include <linux/cdev.h>

static struct class *eep_class = NULL;
static int probed_ndevices = 0;
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);
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
struct eep_dev {
    unsigned char *data;
    struct i2c_client *client;
    struct mutex eep_mutex;
    struct list_head    device_entry;
    dev_t       devt;
    unsigned    users;
    struct cdev eep_cdev;
    struct AT24C256_node AT24C256_node_obj;
    struct regmap *eep_regmap;
};
static int eep_open(struct inode *inode, struct file *filp)
{
    struct eep_dev *eeprom = NULL;
    int ret_value = 0;
    mutex_lock(&device_list_lock);
    ret_value = -ENXIO;
    list_for_each_entry(eeprom, &device_list, device_entry)
    {
        if (eeprom->devt == inode->i_rdev) 
        {
            ret_value = 0;
            break;
        }
    }
    if (ret_value < 0)
    {
        pr_err("AT24C256 driver: no device matched\n");
        goto NO_DEVICE_MATCHED;
    }
    pr_info("AT24C256 driver: character device number matched: %d %d\n", MAJOR(eeprom->devt), MINOR(eeprom->devt));
    /* if opened the 1st time, allocate the buffer */
    if (eeprom->data == NULL) {
        eeprom->data = kzalloc(eeprom->AT24C256_node_obj.bytesize, GFP_KERNEL);
        if (!eeprom->data) {
            pr_err("AT24C256 driver: out of memory when allocating buffer\n");
            ret_value = -ENOMEM;
            goto ALLOCATE_BUFFER_FAILED;
        }
    }
    eeprom->users++;
     /* store a pointer to struct eep_dev here for other methods */
    filp->private_data = eeprom;
    mutex_unlock(&device_list_lock);
    goto OPEN_RETURN_LABEL;
ALLOCATE_BUFFER_FAILED:
    pr_info("AT24C256 driver: ALLOCATE_BUFFER_FAILED\n");
NO_DEVICE_MATCHED:
    mutex_unlock(&device_list_lock);
    pr_info("AT24C256 driver: NO_DEVICE_MATCHED\n");
OPEN_RETURN_LABEL:
pr_info("AT24C256 driver: OPEN_RETURN_LABEL %d\n", ret_value);
    return ret_value;
}

int eep_release(struct inode *inode, struct file *filp)
{
    struct eep_dev *eeprom;
    mutex_lock(&device_list_lock);
    eeprom = filp->private_data;
    /* last close? */
    eeprom->users--;
    if (!eeprom->users) {
        kfree(eeprom->data);
        eeprom->data = NULL ; /* Never forget to set to 'NULL' after freeing */
    }
    mutex_unlock(&device_list_lock);
    pr_info("AT24C256 driver: Released\n");
    return 0;
}
ssize_t eep_read(struct file *filp, char __user *buf,
                    size_t count, loff_t *f_pos)
{
    struct eep_dev *eeprom = filp->private_data;
    ssize_t ret_value = 0;
    if (mutex_lock_killable(&eeprom->eep_mutex))
    {
        ret_value = -EINTR;
        pr_err("AT24C256 driver: cannot lock eep\n");
        goto EEP_MUTEX_LOCK_FAILED;
    }
    if (*f_pos >= eeprom->AT24C256_node_obj.bytesize)
    {
        mutex_unlock(&eeprom->eep_mutex);
        pr_info("AT24C256 driver: f_pos exceeded\n");
        goto FPOS_EXCEEDED;
    } 
    if (*f_pos + count > eeprom->AT24C256_node_obj.bytesize) count = eeprom->AT24C256_node_obj.bytesize - *f_pos;
    if (count > eeprom->AT24C256_node_obj.bytesize) count = eeprom->AT24C256_node_obj.bytesize;
    if ((ret_value = regmap_bulk_read(eeprom->eep_regmap, (u16)(*f_pos), eeprom->data, count)) < 0)
    {
        pr_err("AT24C256 driver: regmap_bulk_read failed: %d\n", ret_value);
        goto I2C_BULK_READ_FAILED;
    };
    //mdelay(5);
    pr_info("AT24C256 driver: read i2c data %s\n", eeprom->data);
    if (copy_to_user(buf, eeprom->data, count) != 0) 
    {
        ret_value = -EIO;
        mutex_unlock(&eeprom->eep_mutex);
        pr_err("AT24C256 driver: copy to user failed\n");
        goto COPY_TO_USER_FAILED;
    }
    ret_value = count;
    *f_pos += count;
    mutex_unlock(&eeprom->eep_mutex);
    goto READ_RETURN_LABEL;
COPY_TO_USER_FAILED:
    pr_info("AT24C256 driver: COPY_TO_USER_FAILED\n");
I2C_BULK_READ_FAILED:
    pr_info("AT24C256 driver: I2C_BULK_READ_FAILED\n");
FPOS_EXCEEDED:
    pr_info("AT24C256 driver: FPOS_EXCEEDED\n");
EEP_MUTEX_LOCK_FAILED:
    pr_info("AT24C256 driver: EEP_MUTEX_LOCK_FAILED\n");
READ_RETURN_LABEL:
    pr_info("AT24C256 driver: READ_RETURN_LABEL %d\n", ret_value);
    return ret_value;
}

struct file_operations eep_fops = {
	.owner =    THIS_MODULE,
 	.read =     eep_read,
// 	.write =    eep_write,
 	.open =     eep_open,
 	.release =  eep_release,
// 	.llseek =   eep_llseek,
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
    int ret_value = 0;
    struct device *device = NULL;
    struct eep_dev *eeprom = NULL;
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
        goto PROBED_RETURN_LABEL;
    }
    pr_info("AT24C256 driver: Executing device driver matching...\n");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        ret_value =  -EIO;
        goto I2C_CHECK_FAILED;
    }  
    //init eeprom struct instance
    eeprom = devm_kzalloc(&client->dev, sizeof(*eeprom), GFP_KERNEL);
    if (!eeprom)
    {
        pr_err("AT24C256 driver: not enough memory to allocate eeprom struct instance\n");
        goto EEPROM_ALLOC_FAILED;
    }
    eeprom->eep_regmap = devm_regmap_init_i2c(client, &AT24Cregmap_config);
    if (IS_ERR(eeprom->eep_regmap))
    {
        pr_err("AT24C256 driver: Cannot init register mapping to i2c device\n");
        ret_value = PTR_ERR(eeprom->eep_regmap);
        goto REGMAP_INIT_FAILED;
    };
    //perform write/read in 0x7fff to test eeprom
    i2c_data = (u32)get_random_u8();
    regmap_write(eeprom->eep_regmap, (u16)0x7fff, (u8)i2c_data);
    pr_info("AT24C256 driver: i2c data to write %d\n", i2c_data);
    mdelay(5);
    regmap_read(eeprom->eep_regmap, (u16)(0x7fff), &i2c_data);
    mdelay(5);
    pr_info("AT24C256 driver: read i2c data %d\n", i2c_data);
    
    memcpy(&eeprom->AT24C256_node_obj, &AT24C256_nd, sizeof(AT24C256_nd));
    ret_value = alloc_chrdev_region(&(eeprom->devt), 0, 1, "eeprom");
    if (ret_value < 0)
    {
        pr_err("AT24C256 driver: Cannot allocate major number\n");
        goto ALLOC_CHAR_DEV_NUM_FAILED;
    }
    cdev_init(&eeprom->eep_cdev, &eep_fops);
    if ((ret_value = cdev_add(&eeprom->eep_cdev, eeprom->devt, 1)) < 0)
    {   
        pr_err("AT24C256 driver: Cannot add the device to the system\n");
        goto ADD_CDEV_FAILED;
    }
    pr_info("AT24C256 driver: Major = %d Minor = %d \n", MAJOR(eeprom->devt), MINOR(eeprom->devt));
    eeprom->data = NULL;
    eeprom->client = client;
    mutex_init(&eeprom->eep_mutex);
    INIT_LIST_HEAD(&eeprom->device_entry);
    device = device_create(eep_class, NULL, eeprom->devt, NULL, "AT24C256" "_%d", probed_ndevices++);
    if (IS_ERR(device)) {
        ret_value = PTR_ERR(device);
        pr_err("AT24C256 driver: Error %d while trying to create AT24C256 device\n", ret_value);
        probed_ndevices --;
        goto CREATE_DEVICE_FAILED;
    }
    mutex_lock(&device_list_lock);
    list_add(&eeprom->device_entry, &device_list);
    i2c_set_clientdata(client, eeprom);
    mutex_unlock(&device_list_lock);

    pr_info("AT24C256 driver: probed, number of device: %d\n", probed_ndevices);
    goto PROBED_RETURN_LABEL;
    //goto CREATE_DEVICE_FAILED;
CREATE_DEVICE_FAILED: 
    pr_info("AT24C256 driver: CREATE_DEVICE_FAILED\n");
    device_destroy(eep_class, eeprom->devt);
ADD_CDEV_FAILED:
    cdev_del(&eeprom->eep_cdev);
ALLOC_CHAR_DEV_NUM_FAILED:
    pr_info("AT24C256 driver: ALLOC_CHAR_DEV_NUM_FAILED\n");
    unregister_chrdev_region(eeprom->devt, 1);
EEPROM_ALLOC_FAILED:
    pr_info("AT24C256 driver: EEPROM_ALLOC_FAILED\n");
    //kfree(eeprom);
REGMAP_INIT_FAILED:
    pr_info("AT24C256 driver: REGMAP_INIT_FAILED\n");
I2C_CHECK_FAILED:
    pr_info("AT24C256 driver: I2C_CHECK_FAILED\n");
PROBED_RETURN_LABEL:
    pr_info("AT24C256 driver: PROBED_RETURN_LABEL %d\n", ret_value);
    return ret_value;
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
    eep_class = class_create(THIS_MODULE, "eeprom1");
     if (IS_ERR(eep_class)) 
     {
        pr_err("AT24C256 driver: error creating eep class\n");
        ret_value = PTR_ERR(eep_class);
        goto EEP_CLASS_CREATE_FAILED;
     };

    if ((ret_value = i2c_register_driver(THIS_MODULE, &AT24C256_i2c_driver)) < 0) 
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
    struct eep_dev *eeprom = NULL;
    mutex_lock(&device_list_lock);
    list_for_each_entry(eeprom, &device_list, device_entry)
    {
        pr_info("AT24C256 driver: destroying & unallocationg char device number %d %d\n", MAJOR(eeprom->devt), MINOR(eeprom->devt));
        device_destroy(eep_class, eeprom->devt);
        unregister_chrdev_region(eeprom->devt, 1);
    }
    i2c_del_driver(&AT24C256_i2c_driver);
    class_destroy(eep_class);
    pr_info("AT24C256 driver: AT24C256_i2c_driver unregistered\n");
}
module_init(AT24C256_driver_init);
module_exit(AT24C256_driver_exit);
MODULE_AUTHOR("dinhtkien");
MODULE_LICENSE("GPL");