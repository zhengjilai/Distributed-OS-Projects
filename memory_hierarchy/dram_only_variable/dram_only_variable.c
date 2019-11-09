#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ktime.h>
#define MAXNUM 100000000

/**
 * Variables `i` and `sum` should be placed in DRAM and never exist in cache.
 * We write a Linux Kernel Module to better manipulate cache.
 * We flush cache by assembly instruction `clflush`.
*/

static inline void clflush_own(volatile void *__q) {
    __asm__ __volatile__(
        "clflush %0" : "+m" (*(volatile char *)__q)
    );
}

// the init function
static int __init dram_only_init(void){
    printk("Enter Linux Kernel, dram variable flush only.\n");

    // time calculation module
    ktime_t start, end;
    s64 actual_time;

    // disable cache before calculation
    start = ktime_get();
 
    int i;
    int sum = 0;
    for (i = 0; i < MAXNUM; i += 4){
        sum++;
        clflush_own(&sum);
        sum++;
        clflush_own(&sum);
        sum++;
        clflush_own(&sum);
        sum++;
        clflush_own(&sum);
        clflush_own(&i);
    }
    printk("Result for dram only (variable) accumulation is %d\n", sum);

    // time calculation module
    end = ktime_get();

    // Use below code for millisec precision
    actual_time = ktime_to_ms(ktime_sub(end, start));
    printk("Time taken for function (dram variable only) execution: %u ms\n", (unsigned int)actual_time);    
    return 0;
}

// the exit function
static void __exit dram_only_cleanup(void){
    printk("DRAM variable flush only: Farewell Linux Kernel.\n");
}

module_init(dram_only_init);
module_exit(dram_only_cleanup);
MODULE_LICENSE("Dual BSD/GPL"); 
