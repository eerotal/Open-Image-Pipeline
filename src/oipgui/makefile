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

CCFLAGS= -Wall -Wpedantic -Wextra -pedantic-errors -std=gnu11 -DOIP_BINARY
LFLAGS=-lm -pthread -loipcore

CCFLAGS_GTK=$(shell pkg-config --cflags gtk+-3.0)
LFLAGS_GTK=$(shell pkg-config --libs gtk+-3.0)

NAME=oipgui

# Enable debugging if DEBUG is set to 1 on the CLI.
DEBUG=0
ifeq ($(DEBUG), 1)
$(info [INFO]: Enabling debugging options.)
CCFLAGS+=-fsanitize=address -g
endif

ifndef BUILDROOT
$(error Buildroot not defined!)
endif

# Setup some path variables.
SRCDIR=$(BUILDROOT)/src/oipgui/src
BINDIR=$(BUILDROOT)/src/oipgui/bin

INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipbuildinfo
INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipimgutil
INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipcore

LIBS+=-L$(BUILDROOT)/src/oipcore/bin

SRCFILES=$(shell find $(SRCDIR) -name *.c -o -name *.h)
HEADERFILES=$(shell find $(BUILDROOT)/src/oipcore -name *.h)
LIBFILES=$(shell find $(BUILDROOT)/src/oipcore -name *.a -o -name *.so)

.PHONY: oipgui clean-all

# Compile the OIP GUI interface.
oipgui: $(BINDIR)/$(NAME).o
$(BINDIR)/$(NAME).o: $(SRCFILES) $(HEADERFILES) $(LIBFILES)
	@echo "[INFO]: Compiling the OIP gui..."
	@mkdir -p $(BINDIR)
	@$(CC) $(CCFLAGS_GTK) -o $(BINDIR)/$(NAME).o $(CCFLAGS)\
		$(SRCFILES) $(INCLUDES) $(LIBS) $(LFLAGS) $(LFLAGS_GTK)
	

# Run clean and clean-modules.
clean-all:
	rm -rf $(BUILDROOT)/src/oipgui/bin
