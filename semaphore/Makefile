BINDIR=bin
.ONESHELL:
SHELL := /bin/bash
.SUFFIXES: .c
CFLAGS = -Wall -g -std=c99 
LIBS = -lrt -pthread

all: clean semaphore

semaphore: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/semaphore.o semaphore.c $(CFLAGS) $(LIBS)

clean:
	rm -r ${BINDIR}
