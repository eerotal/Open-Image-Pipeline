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
CCFLAGS=-Wall -Wpedantic -Wextra -pedantic-errors -std=gnu11 -DOIP_BINARY
LFLAGS=-ldl -lfreeimage -lm -pthread
NAME=oip

DEBUG=0

# Enable debugging if DEBUG is set to 1 on the CLI.
ifeq ($(DEBUG), 1)
$(info [INFO]: Enabling debugging options.)
CCFLAGS+=-fsanitize=address -g
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
	@echo "[INFO]: Compiling main"
	@echo "[INFO]: Submodules are compiled into the main binary automatically."\
		"If you need them as separate libraries, run 'make modules' too."

	@mkdir -p $(BINDIR)
	@mkdir -p $(PLUGINDIR)
	@sh build-config.sh $(DEBUG)

	@echo -n "[INFO]: Compiling Open Image Pipeline..."
	@$(CC) -o $(BINDIR)/$(NAME).o $(CCFLAGS) $(SRCFILES) $(INCLUDES) $(LIBS) $(LFLAGS)
	@echo " Done."

modules: 
	@echo "[INFO]: Compiling submodules..."
	@for DIR in $(SUBMODULES); do\
		test -s $$DIR/makefile && make -C $$DIR DEBUG=$(DEBUG) ;\
	done

clean-modules:
	@echo "[INFO]: Cleaning submodule files."
	@for DIR in $(SUBMODULES); do\
		test -s $$DIR/makefile && make -C $$DIR clean;\
	done

clean:
	rm -rf $(BINDIR)

clean-all: clean clean-modules

LOC:
	wc -l $(SRCFILES)
