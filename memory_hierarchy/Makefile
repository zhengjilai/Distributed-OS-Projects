BINDIR=bin
.ONESHELL:
SHELL := /bin/bash
.SUFFIXES: .c
REG_CFLAGS = -Wall -g -std=c99
CACHE_CFLAGS = -Wall -g -std=c99

all: clean cache register

cache: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/cache_only.o cache_only.c $(CACHE_CFLAGS)

register: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/register_only.o register_only.c $(REG_CFLAGS) 

clean:
	if [ -d "${BINDIR}" ]; then rm -r "${BINDIR}"; fi
	
