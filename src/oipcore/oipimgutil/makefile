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

# Enable debugging if DEBUG is set to true on the CLI.
DEBUG=0
ifeq ($(DEBUG), 1)
$(info [INFO]: Enabling debugging options.)
CCFLAGS+=-fsanitize=address -g
endif

ifndef BUILDROOT
$(error Buildroot not defined!)
endif

# Setup some path variables.
BINDIR=$(BUILDROOT)/src/oipcore/$(NAME)/bin
SRCDIR=$(BUILDROOT)/src/oipcore/$(NAME)

INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipcore

SRCFILES=$(shell find $(SRCDIR) -name *.c -o -name *.h)

compile: $(FILES)
	@echo -n "[INFO]: Compiling the $(NAME) submodule...";	\
	mkdir -p $(BINDIR);					\
	cd $(BINDIR);						\
	$(CC) -c $(CCFLAGS) $(INCLUDES) $(SRCFILES) $(LFLAGS);	\
	ar rcs lib$(NAME).a *.o;				\
	rm *.o;							\
	echo " Done."
	
clean-all:
	rm -rf $(BINDIR)
