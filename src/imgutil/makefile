#
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
#

CC=gcc
CCFLAGS=-Wall -Wpedantic -fPIC -std=gnu11
LFLAGS=-lfreeimage
NAME=oipimgutil

DEBUG=false

# Enable debugging if DEBUG is set to true on the CLI.
ifeq ($(DEBUG), true)
$(info [INFO]: Enabling debugging options.)
CCFLAGS+=-fsanitize=address -g
endif

# Setup some path variables.
BINDIR=bin
BUILDROOT=$(shell pwd)
INCLUDES=-I$(shell dirname $(BUILDROOT))/headers
FILES=$(shell find $(BUILDROOT) -name *.c -o -name *.h)

.ONESHELL:
compile: $(FILES)
	$(info [INFO]: Compiling the imgutil submodule.)
	mkdir -p $(BINDIR)
	cd $(BINDIR)
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(FILES) $(LFLAGS)
	ar rcs lib$(NAME).a *.o
	rm *.o
	
clean:
	rm -rf $(BINDIR)
