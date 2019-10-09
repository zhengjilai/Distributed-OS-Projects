BINDIR=bin
.ONESHELL:
SHELL := /bin/bash
.SUFFIXES: .c
CFLAGS = -Wall -g -std=c99 
LIBS = -lrt -pthread

all: clean semaphore conditional

semaphore: 
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/semaphore.o semaphore.c $(CFLAGS) $(LIBS)

conditional:
	if [ ! -d "${BINDIR}" ]; then mkdir "${BINDIR}"; fi
	gcc -o ${BINDIR}/conditional.o conditionalVariable.c $(CFLAGS) $(LIBS)

clean:
	rm -r ${BINDIR}
