BINDIR=bin
.ONESHELL:
SHELL := /bin/bash
.SUFFIXES: .c
REG_CFLAGS = -Wall -g -std=c99
CACHE_CFLAGS = -Wall -g -std=c99 -O0
CC := gcc
EXTRA_CFLAGS := -Wall -O0
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
	gcc -o ${BINDIR}/register_only.o register_only.c $(REG_CFLAGS) 

dram:
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) modules

clean:
	if [ -d "${BINDIR}" ]; then rm -r ${BINDIR}; fi
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) clean
	if [ -e "dram_only.o.ur-safe" ]; then rm dram_only.o.ur-safe; fi
	
