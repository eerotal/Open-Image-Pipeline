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

# Setup some path variables.
BUILDROOT=$(shell pwd)

DEBUG=0
export DEBUG BUILDROOT

.ONESHELL: all oipcore oipmodules oipshell dirs
.PHONY: all oipcore oipmodules oipshell dirs

# Compile everything.
all: build-config dirs
	@. $(BUILDROOT)/build-config
	make -C "src/oipcore/" oipcore oipmodules
	make -C "src/oipshell/" oipshell

# Compile the OIP core shared library.
oipcore: build-config
	@. $(BUILDROOT)/build-config
	make -C "src/oipcore/" oipcore

# Compile the OIP core submodules.
oipmodules: build-config
	@. $(BUILDROOT)/build-config
	make -C "src/oipcore/" oipmodules

# Compile the OIP shell.
oipshell: build-config
	@. $(BUILDROOT)/build-config
	make -C "src/oipshell/" oipshell

# Create the directory layout needed for running OIP.
dirs:
	@mkdir -p plugins

# Clean the source tree from compilation files.
clean-all:
	@echo "[INFO]: Cleaning all compilation files..."
	rm -rf plugins
	rm -rf cache
	rm -f build-config
	make -C "src/oipcore/" clean-all
	make -C "src/oipshell/" clean-all

# Generate the build-config makefile.
.PHONY: build-config
build-config: config-build.sh
	@sh config-build.sh $(DEBUG)

# Count the lines of code of this project.
LOC:
	wc -l $$(find . -name *.c -o -name *.h)
