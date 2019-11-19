#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

static inline bool gpio_is_valid(int number)            // check validity of GPIO number (max on BBB is 127)
static inline int  gpio_request(unsigned gpio, const char *label)        // allocate the GPIO number, the label is for sysfs
static inline int  gpio_export(unsigned gpio, bool direction_may_change) // make available via sysfs and decide if it can change from input to output and vice versa
static inline int  gpio_direction_input(unsigned gpio)  // an input line (as usual, return of 0 is success)
static inline int  gpio_get_value(unsigned gpio)        // get the value of the GPIO line
static inline int  gpio_direction_output(unsigned gpio, int value)       // value is the state
static inline int  gpio_set_debounce(unsigned gpio, unsigned debounce)   // set debounce time in ms (platform dependent)
static inline int  gpio_sysfs_set_active_low(unsigned gpio, int value)   // set active low (invert operation states)
static inline void gpio_unexport(unsigned gpio)         // remove from sysfs
static inline void gpio_free(unsigned gpio)             // deallocate the GPIO line
static inline int  gpio_to_irq(unsigned gpio)           // associate with an IRQ

static unsigned int gpioLED = 0;       
static bool ledOn = 0; 
static unsigned int blinkDelay = 1000;

struct timer_list my_timer;

static void my_timer_func(struct timer_list *timer) {
    ledOn = ledOn ? 0 : 1;
    gpio_set_value(gpioLED, ledOn);  

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(blinkDelay));
}

static int __init led_init(void) {
    printk(KERN_INFO "Led Blink ON.....\n");

    if (!gpio_is_valid(gpioLED)){
      printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
    }

    gpio_request(gpioLED, "sysfs");
    gpio_direction_output(gpioLED, ledOn);
    pio_export(gpioLED, false);   

    timer_setup(&my_timer, my_timer_func, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(blinkDelay));

    return 0;
}

static void __exit led_cleanup(void) {
    printk(KERN_INFO "Led Blink OFF...\n");

    gpio_set_value(gpioLED, 0);
    gpio_unexport(gpioLED); 
    gpio_free(gpioLED);

    del_timer(&my_timer);
}

module_init(led_init);
module_exit(led_cleanup);
