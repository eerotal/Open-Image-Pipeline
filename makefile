#
#  Copyright 2017 Eero Talus
#
#  This file is part of Open Image Pipeline.
#
#  Open Image Pipeline is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Open Image Pipeline is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Open Image Pipeline.  If not, see <http://www.gnu.org/licenses/>.
#

CC=gcc
CCFLAGS=-Wall -Wpedantic -std=gnu11 -g -DOIP_BINARY
LFLAGS=-ldl -lfreeimage -lm -pthread
NAME=oip

MEMDEBUG=false

# Enable memory debugging if MEMDEBUG is set to true on the CLI.
ifeq ($(MEMDEBUG), true)
$(info [INFO]: Enabling memory debugging options.)
CCFLAGS+=-fsanitize=address
endif

# Setup some path variables.
SRCDIR=src
BINDIR=bin
PLUGINDIR=plugins
BUILDROOT=$(shell pwd)

INCLUDES=-Isrc/imgutil -Isrc/headers
SRCFILES=$(shell find $(BUILDROOT)/$(SRCDIR) -name *.c -o -name *.h)

SUBMODULES=$(shell ls -d $(SRCDIR)/*/)

main: $(SRCFILES)
	$(info [INFO]: Compiling main)
	$(info [INFO]: Submodules are compiled into the main binary automatically.\
	If you need them as separate libraries, run 'make modules' too.)

	mkdir -p $(BINDIR)
	mkdir -p $(PLUGINDIR)
	$(CC) -o $(BINDIR)/$(NAME).o $(CCFLAGS) $(SRCFILES) $(INCLUDES) $(LIBS) $(LFLAGS)

modules: 
	$(info [INFO]: Compiling modules)
	for DIR in $(SUBMODULES); do\
		test -s $$DIR/makefile && make -C $$DIR MEMDEBUG=$(MEMDEBUG) ;\
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
