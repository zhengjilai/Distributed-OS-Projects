BINDIR=bin
.ONESHELL:
SHELL := /bin/bash
.SUFFIXES: .c
REG_CFLAGS = -Wall -g -std=c99
CACHE_CFLAGS = -Wall -g -std=c99 -O0
DRAM_CFLAGS = -Wall 
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
obj-m := dram_only.o
module-objs := dram_only.o

all: clean cache register dram

cache: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/cache_only.o cache_only.c $(CACHE_CFLAGS)

register: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc register_only.c $(REG_CFLAGS) -o ${BINDIR}/register_only.o 

dram:
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	cd $(BINDIR)
	make $(DRAM_CFLAGS) -C $(KDIR) M=$(PWD) modules
	cd ..

clean:
	if [ -d "${BINDIR}" ]; then rm -r ${BINDIR}; fi
	make $(DRAM_CFLAGS) -C $(KDIR) M=$(PWD) clean
	if [ -e "dram_only.o.ur-safe" ]; then rm dram_only.o.ur-safe; fi
	
