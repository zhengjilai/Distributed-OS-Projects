#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ktime.h>
#include <linux/smp.h>
#define MAXNUM 100000000

/**
 * Variables `i` and `sum` should be placed in DRAM and never exist in cache.
 * We write a Linux Kernel Module to better manipulate cache.
 * We disable the cache my configuring the value of register `cr0`.
 * We flush cache by assembly instruction `wbinvd`.
*/
// value for cr0 register
unsigned long cr0;

// disable cache by setting cr0 (DC to 1 and NW to 0)
static inline void manipulate_cr0_for_cache(void *enable) {
    if (enable){
        // cr0 = read_cr0();
	    // write_cr0(cr0 & ~0x40000000);
	    __asm__(
			"push %%rax\n\t"
			"mov %%cr0,%%rax\n\t"
			"and $(~(1<<30)),%%rax\n\t"
			"mov %%rax,%%cr0\n\t"
			"wbinvd\n\t"
			"pop %%rax\n\t"
            ::: "%rax"
        );
    } else {
        // cr0 = read_cr0();
        // write_cr0(cr0 | 0x40000000);
        // __asm__ __volatile__(
        //     "wbinvd \n\t"
        //     :::"memory"
        // );
	    __asm__(
			"push %%rax\n\t"
			"mov %%cr0,%%rax\n\t"
			"or $(1<<30),%%rax\n\t"
			"mov %%rax,%%cr0\n\t"
			"wbinvd\n\t"
			"pop %%rax\n\t"
            ::: "%rax"
        );
    }
}

// Calls manipulate_cr0_for_cache to disable/enable cache accross all cores
void smp_sysmem_cache_manipulation(int enable) {
	manipulate_cr0_for_cache((void *) enable);
	smp_call_function(manipulate_cr0_for_cache, (void *) enable, true);
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
    smp_sysmem_cache_manipulation(0);
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
    }
    printk("Result for dram only accumulation is %d\n", sum);

    // time calculation module
    end = ktime_get();

    // enable cache after calculation
    cr0 = read_cr0();
    printk("cr0 %lx \n", cr0); 
    smp_sysmem_cache_manipulation(1);
    cr0 = read_cr0();
    printk("cr0 %lx \n", cr0); 

    // Use below code for millisec precision
    actual_time = ktime_to_ms(ktime_sub(end, start));
    printk("Time taken for function (disable whole cache) execution: %u ms\n", (unsigned int)actual_time);    
    return 0;
}

// the exit function
static void __exit dram_only_cleanup(void){
    printk("DRAM only, disable whole cache: Farewell Linux Kernel.\n");
}

module_init(dram_only_init);
module_exit(dram_only_cleanup);
MODULE_LICENSE("Dual BSD/GPL"); 
