CC=gcc
CCFLAGS=-Wall -Wpedantic -std=gnu99
LFLAGS=-ldl -lfreeimage
NAME=oip

SRCDIR=src
BINDIR=bin
BUILDROOT=$(shell pwd)

INCLUDES=-Isrc/imgutil -Isrc/oip
SRCFILES=$(shell find $(BUILDROOT)/$(SRCDIR) -name *.c -o -name *.h)

SUBMODULES=$(shell ls -d $(SRCDIR)/*/)

main: $(SRCFILES)
	$(info ==== COMPILING MAIN ====)
	$(info [INFO]: Submodules are compiled into the main binary automatically.\
	If you need them as separate libraries, run 'make modules' too.)

	$(CC) -o $(BINDIR)/$(NAME).o $(CCFLAGS) $(SRCFILES) $(INCLUDES) $(LIBS) $(LFLAGS)

modules: 
	$(info ==== COMPILING SUBMODULES ====)
	for DIR in $(SUBMODULES); do\
		make -C $$DIR;\
	done
