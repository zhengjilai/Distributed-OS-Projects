BINDIR=bin
.ONESHELL:
SHELL := /bin/bash
.SUFFIXES: .c
CFLAGS = -Wall -g -std=c99
CACHEFLAGS = -O0

all: clean cache register

cache: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/cache_only.o cache_only.c $(CFLAGS) $(CACHEFLAGS)

register: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc register_only.c $(CFLAGS) -o ${BINDIR}/register_only.o 

clean:
	if [ -d "${BINDIR}" ]; then rm -r ${BINDIR}; fi
	
