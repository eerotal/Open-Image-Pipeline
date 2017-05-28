CC=gcc
CCFLAGS=-Wall -Wpedantic -std=gnu99 -g
LFLAGS=-ldl -lfreeimage -lm -pthread
NAME=oip

SRCDIR=src
BINDIR=bin
PLUGINDIR=plugins
BUILDROOT=$(shell pwd)

INCLUDES=-Isrc/imgutil -Isrc/oip
SRCFILES=$(shell find $(BUILDROOT)/$(SRCDIR) -name *.c -o -name *.h)

SUBMODULES=$(shell ls -d $(SRCDIR)/*/)

main: $(SRCFILES)
	$(info ==== COMPILING MAIN ====)
	$(info [INFO]: Submodules are compiled into the main binary automatically.\
	If you need them as separate libraries, run 'make modules' too.)

	mkdir -p $(BINDIR)
	mkdir -p $(PLUGINDIR)
	$(CC) -o $(BINDIR)/$(NAME).o $(CCFLAGS) $(SRCFILES) $(INCLUDES) $(LIBS) $(LFLAGS)

modules: 
	$(info ==== COMPILING SUBMODULES ====)
	for DIR in $(SUBMODULES); do\
		test -s $$DIR/makefile && make -C $$DIR;\
	done

clean-modules:
	$(info ==== CLEANING SUBMODULES ====)
	for DIR in $(SUBMODULES); do\
		test -s $$DIR/makefile && make -C $$DIR clean;\
	done

clean:
	rm -rf $(BINDIR)

clean-all: clean clean-modules

LOC:
	wc -l $(SRCFILES)
