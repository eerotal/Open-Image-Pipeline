CC=gcc
CCFLAGS=-Wall -Wpedantic -std=gnu99
LFLAGS=-lfreeimage
NAME=oipimgutil

BINDIR=bin
BUILDROOT=$(shell pwd)
INCLUDES=-I$(BUILDROOT)/imgutil
FILES=$(shell find $(BUILDROOT) -name *.c -o -name *.h)

.ONESHELL:
compile: $(FILES)
	$(info ======= COMPILING IMGUTIL SUBMODULE ======== )
	mkdir -p $(BINDIR)
	cd $(BINDIR)
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(FILES) $(LFLAGS)
	ar rcs lib$(NAME).a *.o
	rm *.o
	
