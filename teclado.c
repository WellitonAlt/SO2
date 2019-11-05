#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/vt_kern.h>

MODULE_LICENSE("GPL");

#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

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
struct timer_list my_timer;
struct tty_driver *my_driver;

int blinkDelay = 1000;
int isBlinking = 0;
int on = 0;

static ssize_t ledBlink_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[maxdatalen];
    long period;

    if (count < maxdatalen) maxdatalen = count;

    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied == 0) {
        databuf[maxdatalen] = 0;
	    if (kstrtol(databuf, 10, &period) > 0 ) return -EINVAL;
	    if (period >= 0 && period < 10)  return -EINVAL;
        if (period < 0) {
            printk("Disable Led Blink");
	    isBlinking = 0;
	    return count;
        }

	blinkDelay = period;
        printk("Period: %d\n", blinkDelay);
        isBlinking = 1;
    } else {
        printk("Period Erro: \n");
        isBlinking = 0;
    }

    return count;
}

static int ledBlink_open(struct inode *inode, struct file *file) {
    printk("Led Blink: Device open\n");
    return 0;
}

static int ledBlink_release(struct inode *inode, struct file *file) {
    printk("Led Blink: Device close\n");
    return 0;
}

static void my_timer_func(struct timer_list *timer) {
    int pstatus;

    if (isBlinking == 1) {
    	if (on == 1) {
	       pstatus = RESTORE_LEDS;
	       on = 0;
	    } else {
	       pstatus = ALL_LEDS_ON;
	       on = 1;
	    }
    } else {
        pstatus = RESTORE_LEDS;
    }

    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, pstatus);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(blinkDelay));
}

static int __init led_init(void) {
    int err; 
    dev_t dev;

    printk(KERN_INFO "Led Blink ON.....\n");

    err = alloc_chrdev_region(&dev, 0, 1, "ledBlink");
    dev_major = MAJOR(dev);
    ledBlink_class = class_create(THIS_MODULE, "ledBlink");

    cdev_init(&ledBlink_data.cdev, &ledBlink_fops);
    ledBlink_data.cdev.owner = THIS_MODULE;
    cdev_add(&ledBlink_data.cdev, MKDEV(dev_major, 0), 1);
    device_create(ledBlink_class, NULL, MKDEV(dev_major, 0), NULL, "ledBlink-0");

    my_driver = vc_cons[fg_console].d->port.tty->driver;
    timer_setup(&my_timer, my_timer_func, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(blinkDelay));

    return 0;
}

static void __exit led_cleanup(void) {
    printk(KERN_INFO "Led Blink OFF...\n");

    device_destroy(ledBlink_class, MKDEV(dev_major, 0));

    class_unregister(ledBlink_class);
    class_destroy(ledBlink_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
    del_timer(&my_timer);
}

module_init(led_init);
module_exit(led_cleanup);


