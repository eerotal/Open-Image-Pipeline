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

.ONESHELL: all oipcore oipmodules oipshell
.PHONY: all oipcore oipmodules oipshell

all: build-config
	@. $(BUILDROOT)/build-config
	make -C "oipcore/" oipcore oipmodules
	make -C "oipshell/" oipshell

oipcore: build-config
	@. $(BUILDROOT)/build-config
	make -C "oipcore/" oipcore

oipmodules: build-config
	@. $(BUILDROOT)/build-config
	make -C "oipcore/" oipmodules

oipshell: build-config
	@. $(BUILDROOT)/build-config
	make -C "oipshell/" oipshell

# Run clean and clean-modules.
clean-all:
	@make -C "oipcore/" clean-all
	@make -C "oipshell/" clean-all

# Generate the build-config makefile.
.PHONY: build-config
build-config: config-build.sh
	@sh config-build.sh $(DEBUG)
