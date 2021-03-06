## Memory Access Hierarchy
This module is written to have deeper insight into memory hierarchy.
The three `*.c` files has the same function, which is incrementing variables `i` and `sum` from `0` to `MAXNUM`. However, variables `i` and `sum` should be placed in different levels of memory, including Register, Cache and DRAM.

### Environment
- Ubuntu 18.04, kernel 5.0.0-32-generic
- gcc, version 7.4.0
- GNU Make, version 4.1
- A Physical machine 
  - Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz
  - 32G DRAM
  - Note: `dram_only_whole.ko` does not work perfectly on a virtual machine !!

### Register Only Access
1. Requirements
- Variables `i` and `sum` should be placed in registers.
  
2. Details
- We insert assembly codes in C to enforce `i` and `sum` to become register variables.
- In our physical machine, the running time for `MAXNUM=100000000` is about *24ms*.

3. Usage
- Use the following command to run it.
```bash
$ make 
$ ./bin/register_only.o
```

### Cache Only Access
1. Requirements
- Variables `i` and `sum` should be placed in cache.
  
2. Details
- In a C program, by default, the varibales are placed in cache.
- We use tag `-O0` to disable gcc optimization.
- In our physical machine, the running time for `MAXNUM=100000000` is about *127ms*.

3. Usage
- Use the following command to run it.
```bash
$ make 
$ ./bin/cache_only.o
```

### DRAM Only Access (Disable whole Cache)
1. Requirements
- Variables `i` and `sum` should be placed in DRAM and never exist in cache.

2. Details
- We write a Linux Kernel Module to better manipulate cache.
- We disable the whole cache by configuring the value of register `cr0`.
- We flush the whole cache after disabling it by assembly instruction `wbinvd`.
- We use tag `-O0` to disable gcc optimization.
- In our physical machine, the running time for `MAXNUM=100000000` is about *143000ms*, even our mouse and keyboard cannot function smoothly during that time.
- However, in a VMWare Workstation virtual machine, the running time for `MAXNUM=100000000` is about *131ms*, almost the same as `cache_only.o`. We ascribe it to the actual realization of virtual machine.

3. Usage
- Use the following command to run it.
```bash
$ cd dram_only_whole
$ make 
$ sudo insmod dram_only_whole.ko
$ sudo rmmod dram_only_whole
$ dmesg | tail -n 20
```

### DRAM Only Access (Flush Cache Line)
1. Requirements
- Variables `i` and `sum` should be placed in DRAM and never exist in cache.

2. Details
- We write a Linux Kernel Module to better manipulate cache.
- We flush the cache lines for variables `i` and `sum` by assembly instruction `clflush`.
- We use tag `-O0` to disable gcc optimization.
- In our physical machine, the running time for `MAXNUM=100000000` is about *11785ms*, much shorter than the time of disabling the whole cache through register `cr0`.

3. Usage
- Use the following command to run it.
```bash
$ cd dram_only_variable
$ make 
$ sudo insmod dram_only_variable.ko
$ sudo rmmod dram_only_variable
$ dmesg | tail -n 20
```