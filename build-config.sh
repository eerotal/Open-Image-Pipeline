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

#!/bin/sh

HEADERS_DIR="src/headers"
TEMPLATES_DIR="templates"

BUILD_VERSION=$(git describe --always --tags --dirty)
BUILD_DATE=$(date)
BUILD_DEBUG=$1

if [ "$BUILD_DEBUG" != "1" ] && [ "$BUILD_DEBUG" != "0" ]; then
	echo "[ERROR]: Debug build parameter not specified. Exiting."
	exit 1
fi

echo "\n[INFO]: Version info that will be compiled into the binary: "
echo "[INFO]: \tVersion:     "$BUILD_VERSION
echo "[INFO]: \tDate:        "$BUILD_DATE
echo "[INFO]: \tDebug build: "$BUILD_DEBUG"\n"

REPL_VERSION='s/<BUILD_VERSION>/"'$BUILD_VERSION'"/g'
REPL_DATE='s/<BUILD_DATE>/"'$BUILD_DATE'"/g'
REPL_DEBUG='s/<BUILD_DEBUG>/'$BUILD_DEBUG'/g'

echo -n "[INFO]: Generating the build.h header file... "

cp $TEMPLATES_DIR"/build.h" $HEADERS_DIR"/build.h"
sed -i "$REPL_VERSION" $HEADERS_DIR/build.h
sed -i "$REPL_DATE" $HEADERS_DIR/build.h
sed -i "$REPL_DEBUG" $HEADERS_DIR/build.h

echo "Done."
