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
LFLAGS=-lm -pthread -loipcore
NAME=oipshell

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
SRCDIR=$(BUILDROOT)/oipshell/src
BINDIR=$(BUILDROOT)/oipshell/bin

INCLUDES+=-I$(BUILDROOT)/oipcore/src/oipbuildinfo
INCLUDES+=-I$(BUILDROOT)/oipcore/src/oipimgutil
INCLUDES+=-I$(BUILDROOT)/oipcore/src/oip

LIBS+=-L$(BUILDROOT)/oipcore/bin

SRCFILES=$(shell find $(SRCDIR) -name *.c -o -name *.h)

# Compile the OIP shell interface.
oipshell:
	@echo "[INFO]: Compiling OIP shell..."

	@mkdir -p $(BINDIR)
	@$(CC) -o $(BINDIR)/$(NAME).o $(CCFLAGS) $(SRCFILES) $(INCLUDES) $(LIBS) $(LFLAGS)
	

# Run clean and clean-modules.
clean-all:
	Â@echo "!! TODO !!"

