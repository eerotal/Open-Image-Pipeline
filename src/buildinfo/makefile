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
LFLAGS=
NAME=oipbuildinfo

# Enable debugging if DEBUG is set to true on the CLI.
DEBUG=0
ifeq ($(DEBUG), 1)
$(info [INFO]: Enabling debugging options.)
CCFLAGS+=-fsanitize=address -g
endif

# Setup some path variables.
BINDIR=bin
BUILDROOT=$(shell pwd)
INCLUDES=-I$(shell dirname $(BUILDROOT))/headers
FILES=$(shell find $(BUILDROOT) -name *.c -o -name *.h)

ifndef OIP_VERFLAGS
$(error Can't compile the buildinfo submodule because OIP_VERFLAGS is not defined.)
endif

compile: $(FILES)
	@echo -n "[INFO]: Compiling the buildinfo submodule...";	\
	mkdir -p $(BINDIR);						\
	cd $(BINDIR);							\
	$(CC) -c $(CCFLAGS)						\
		-DOIP_BUILD_VERSION=$$OIP_BUILD_VERSION			\
		-DOIP_BUILD_DATE=$$OIP_BUILD_DATE			\
		-DOIP_BUILD_DEBUG=$$OIP_BUILD_DEBUG			\
		$(INCLUDES) $(FILES) $(LFLAGS);				\
	ar rcs lib$(NAME).a *.o;					\
	rm *.o;								\
	echo " Done."
	
clean:
	rm -rf $(BINDIR)
