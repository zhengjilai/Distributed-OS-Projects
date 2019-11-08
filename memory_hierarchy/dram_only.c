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
static int __init dram_only_init(void){
    printk("Enter Linux Kernel.\n");

    // time calculation module
    ktime_t start, end;
    s64 actual_time;

    // disable cache before calculation
    cr0 = read_cr0();
    printk("cr0 %lx \n", cr0); 
    disable_cache();
    cr0 = read_cr0();
    printk("cr0 %lx \n", cr0); 
    start = ktime_get();
    int i;
    int sum = 0;
    for (i = 0; i < MAXNUM; i += 4){
        sum++;
        sum++;
        sum++;
        sum++;
        // if ((i % 10000000) == 0) printk("now it is %d iteration\n", i);
    }
    printk("Result for cache only accumulation is %d\n", sum);

    // enable cache after calculation
    cr0 = read_cr0();
    printk("cr0 %lx \n", cr0); 
    enable_cache();
    cr0 = read_cr0();
    printk("cr0 %lx \n", cr0); 

    // time calculation module
    end = ktime_get();
    // Use below code for millisec precision
    actual_time = ktime_to_ms(ktime_sub(end, start));
    printk("Time taken for function execution: %u ms\n", (unsigned int)actual_time);    
    return 0;
}

// the exit function
static void __exit dram_only_cleanup(void){
    printk("DRAM only: Farewell Linux Kernel.\n");
}

module_init(dram_only_init);
module_exit(dram_only_cleanup);
MODULE_LICENSE("Dual BSD/GPL"); 
