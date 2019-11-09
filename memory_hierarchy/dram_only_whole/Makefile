CC := gcc
EXTRA_CFLAGS := -Wall -O0
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
obj-m := dram_only_whole.o
module-objs := dram_only_whole.o

all: clean dram_whole

dram_whole:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) clean
	if [ -e "dram_only_whole.o.ur-safe" ]; then rm dram_only_whole.o.ur-safe; fi