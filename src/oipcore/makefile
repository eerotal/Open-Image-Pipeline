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
CCFLAGS=-Wall -Wpedantic -Wextra -pedantic-errors -shared -fPIC -std=gnu11 -DOIP_BINARY
LFLAGS=-ldl -lfreeimage -lm
NAME=oipcore

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
SRCDIR=$(BUILDROOT)/src/oipcore
BINDIR=$(BUILDROOT)/src/oipcore/bin

INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipimgutil
INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipbuildinfo
INCLUDES+=-I$(BUILDROOT)/src/oipcore/oipcore

SRCFILES=$(shell find $(SRCDIR) -name *.c -o -name *.h)
SUBMODULES=$(shell ls -d $(SRCDIR)/*)

.PHONY: oipcore oipmodules clean-all
.ONESHELL: oipcore oipmodules oipshell

# Compile the OIP Core library.
oipcore: $(BINDIR)/lib$(NAME).so
$(BINDIR)/lib$(NAME).so: $(SRCFILES)
	@mkdir -p $(BINDIR)
	@echo -n "[INFO]: Compiling the OIP core..."
	@$(CC) -o $(BINDIR)/lib$(NAME).so $(CCFLAGS)		\
		-DOIP_BUILD_VERSION=$$OIP_BUILD_VERSION		\
		-DOIP_BUILD_DATE=$$OIP_BUILD_DATE		\
		-DOIP_BUILD_DEBUG=$$OIP_BUILD_DEBUG		\
		-DOIP_BUILD_ABI=$$OIP_BUILD_ABI			\
		$(SRCFILES) $(INCLUDES) $(LFLAGS)
	@echo " Done."

# Compile all the OIP Core submodules.
oipmodules:
	@echo "[INFO]: Compiling the OIP submodules..."
	for DIR in $(SUBMODULES); do
		if [ -f $$DIR/makefile ] && [ -f $$DIR/submodule ]; then
			make -C $$DIR
		fi
	done

# Cleanup all compilation files.
clean-all:
	@rm -rf $(BINDIR)
	echo "[INFO]: Cleaning all submodule files..."
	for DIR in $(SUBMODULES); do
		if [ -f $$DIR/makefile ] && [ -f $$DIR/submodule ]; then
			make -C $$DIR clean-all
		fi
	done
