#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ktime.h>
#define MAXNUM 100000000

// value for cr0 register
unsigned long cr0;

// disable cache by cr0
static inline void disable_cache(void)
{
	write_cr0(cr0 | 0x40000000);
}

// enable cache by cr0
static inline void enable_cache(void)
{
	write_cr0(cr0 & ~0x40000000);
}

// the init function
static int __init cache_manipulation_init(void){
    printk("Enter Linux Kernel.\n");

    // time calculation module
    ktime_t start, end;
    s64 actual_time;
    start = ktime_get();

    cr0 = read_cr0();
    disable_cache();
    int i;
    int sum = 0;
    for (i = 0; i < MAXNUM; i += 4){
        sum++;
        sum++;
        sum++;
        sum++;
    }
    printk("Result for cache only accumulation is %d\n", sum);

    // time calculation module
    end = ktime_get();
    // Use below code for millisec precision
    actual_time = ktime_to_ms(ktime_sub(end, start));
    printk("Time taken for function() execution: %u\n", (unsigned int)actual_time);    
}

// the exit function
static void __exit cache_manipulation_cleanup(void){
    cr0 = read_cr0();
    enable_cache();
    printk("Farewell Linux Kernel.\n");
}

module_init(cache_manipulation_init);
module_exit(cache_manipulation_cleanup);