#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

static unsigned int gpioLED = 17;       
static bool ledOn = 0; 
static unsigned int blinkDelay = 500;

struct timer_list my_timer;

static void my_timer_func(struct timer_list *timer) {
    ledOn = ledOn ? 0 : 1;
    printk(KERN_INFO "Led Aqui2 %d \n", ledOn);
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
    gpio_export(gpioLED, false);   

    //gpio_set_value(gpioLED, 1);

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
