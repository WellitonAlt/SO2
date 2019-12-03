#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/configfs.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

static int ledBlink_open(struct inode *inode, struct file *file);
static int ledBlink_release(struct inode *inode, struct file *file);
static ssize_t ledBlink_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations ledBlink_fops = {
    .owner      = THIS_MODULE,
    .open       = ledBlink_open,
    .release    = ledBlink_release,
    .write       = ledBlink_write
};

static int dev_major = 0;
static struct class *ledBlink_class = NULL;

struct ledBlink_device_data ledBlink_data;
struct ledBlink_device_data {
   struct cdev cdev;
};

static unsigned int gpioLED = 17;       
static bool ledOn = 0;
static bool isBlinking = 0;
static unsigned int blinkDelay = 1000;

struct timer_list my_timer;

static ssize_t ledBlink_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[maxdatalen];
    long period;

    if (count < maxdatalen) maxdatalen = count;

    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied == 0) {
        databuf[maxdatalen] = 0;
	    if (kstrtol(databuf, 10, &period) > 0 ) return -EINVAL;
	    
        
        if (period >= 0 && period < 10) {
            printk(KERN_INFO "LedBlink - ON \n");
            ledOn = 1;
            isBlinking = 0;
            return count;
        }
        
        
        if (period < 0) {
            printk(KERN_INFO "LedBlink - O \n");
            ledOn = 0;
	        isBlinking = 0;
	        return count;
        }

	    blinkDelay = period;
        printk(KERN_INFO "LedBlink - Period: %d\n", blinkDelay);
        isBlinking = 1;

        return count;
    }

    printk(KERN_INFO "LedBlink - erro no argumento \n");
    isBlinking = 0; 

    return count;
}

static int ledBlink_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "LedBlink - Device open \n");
    return 0;
}

static int ledBlink_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "LedBlink -  Device close \n");
    return 0;
}

static void my_timer_func(struct timer_list *timer) {
    if (isBlinking == 1) {
        ledOn = ledOn ? 0 : 1;
    }

    gpio_set_value(gpioLED, ledOn);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(blinkDelay));
}

static int __init led_init(void) {
    int err; 
    dev_t dev;

    printk(KERN_INFO "LedBlink - ON.....\n");

    if (!gpio_is_valid(gpioLED)){
      printk(KERN_INFO "LedBlink - GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
    }

    err = alloc_chrdev_region(&dev, 0, 1, "ledBlink");
    dev_major = MAJOR(dev);
    ledBlink_class = class_create(THIS_MODULE, "ledBlink");

    cdev_init(&ledBlink_data.cdev, &ledBlink_fops);
    ledBlink_data.cdev.owner = THIS_MODULE;
    cdev_add(&ledBlink_data.cdev, MKDEV(dev_major, 0), 1);
    device_create(ledBlink_class, NULL, MKDEV(dev_major, 0), NULL, "ledBlink-0");

    gpio_request(gpioLED, "sysfs");
    gpio_direction_output(gpioLED, ledOn);
    gpio_export(gpioLED, false);   

    timer_setup(&my_timer, my_timer_func, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(blinkDelay));

    return 0;
}

static void __exit led_cleanup(void) {
    printk(KERN_INFO "LedBlink - OFF...\n");

    device_destroy(ledBlink_class, MKDEV(dev_major, 0));

    class_unregister(ledBlink_class);
    class_destroy(ledBlink_class);
    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

    gpio_set_value(gpioLED, 0);
    gpio_unexport(gpioLED); 
    gpio_free(gpioLED);

    del_timer(&my_timer);
}

module_init(led_init);
module_exit(led_cleanup);

