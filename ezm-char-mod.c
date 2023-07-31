#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "ezm-char-mod"
#define CLASS_NAME DEVICE_NAME"_class"

#define BUFFER_SIZE 255

static unsigned int major;
static struct class * dummy_class;
static struct cdev dummy_cdev;

static uint8_t buffer[BUFFER_SIZE] = {0};

int dummy_open(struct inode * inode, struct file * filp){
    pr_info("Someone tried to open me\n");
    return 0;
}

int dummy_release(struct inode * inode, struct file * filp){
    pr_info("Someone tried to open me\n");
    return 0;
}

ssize_t dummy_read(struct file * filp, char __user * buf, size_t count, loff_t * offset){
    pr_info("Now I can read from offset [%d] count [%d]\n", *offset, count);
    const uint32_t filesize = BUFFER_SIZE;
    if(*offset >= filesize) return 0;
    if((*offset + count) > filesize)
        count = filesize - (*offset);

    void * from  = buffer + (*offset);

    int sent = copy_to_user(buf, from, count);
    if(sent) return -EFAULT;

    *offset += count;
    return count;
}

ssize_t dummy_write(struct file * file, const char __user * buf, size_t count, loff_t * offset){
    pr_info("Now I accepting data. Writing from offset [%d] count [%d]\n", *offset, count);
    const uint32_t filesize = BUFFER_SIZE;
    if(*offset > filesize) return -EINVAL;
    if((*offset + count) > filesize)
        count = filesize - (*offset);

    if(copy_from_user(buffer, buf, count) != 0){
        return -EFAULT;
    }

    *offset += count;  
    return count;
}

struct file_operations dummy_fops = {
    open:       dummy_open,
    release:    dummy_release,
    read:       dummy_read,
    write:      dummy_write,
};

static int __init dummy_char_init_module(void){
    struct device * dummy_device;
    int error = 0;
    dev_t devt = 0;

    /*Get a range of minor number (starting with 0) to work with*/
    error = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if(error < 0){
        pr_err("Can not get major number\n");
        return error;
    }

    major = MAJOR(devt);
    pr_info("ezm_char major number = %d\n", major);

    /*Create device class, visible in /sys/class */
    dummy_class = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(dummy_class)){
        pr_err("Error creating ezm_char_mod class.\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return PTR_ERR(dummy_class);
    }

    /*Initialize the char device and tie a file_operations to it */
    cdev_init(&dummy_cdev, &dummy_fops);
    dummy_cdev.owner = THIS_MODULE;
    /*Now make the device live for the users to access*/
    cdev_add(&dummy_cdev, devt, 1);

    dummy_device = device_create(
        dummy_class,
        NULL,   /*No parent device*/
        devt,   /*Associated dev_t*/
        NULL,   /*No additional data*/
        DEVICE_NAME /*Device name*/
    );

    if(IS_ERR(dummy_device)){
        pr_err("Error creating ezm_char device.\n");
        class_destroy(dummy_class);
        unregister_chrdev_region(devt,1);
        return PTR_ERR(dummy_device);
    }

    pr_info("Easymetering char module loaded.\n");
    error = 0;
    return error;
}

static void __exit dummy_char_cleanup_module(void){
    unregister_chrdev_region(MKDEV(major, 0), 1);
    device_destroy(dummy_class, MKDEV(major,0));
    cdev_del(&dummy_cdev);
    class_destroy(dummy_class);

    pr_info("Chester says \"Goodbye\". Unloaded module");
}


module_init(dummy_char_init_module);
module_exit(dummy_char_cleanup_module);

MODULE_AUTHOR("Guido Ramirez");
MODULE_LICENSE("GPL");
