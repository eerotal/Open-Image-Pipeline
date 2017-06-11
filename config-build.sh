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

CONFIG_FILE="build-config"

BUILD_VERSION=$(git describe --always --tags --dirty)
BUILD_DATE=$(date --utc|sed 's/ /-/g')
BUILD_DEBUG=$1
BUILD_ABI=$(cat abi-version|sed 's/\s//g')

if [ "$BUILD_DEBUG" != "1" ] && [ "$BUILD_DEBUG" != "0" ]; then
	echo "[ERROR]: Debug build parameter not specified. Exiting."
	exit 1
fi

echo "\n[INFO]: Version info that will be compiled into binaries: "
echo "[INFO]: \tVersion:     "$BUILD_VERSION
echo "[INFO]: \tDate:        "$BUILD_DATE
echo "[INFO]: \tABI version: "$BUILD_ABI
echo "[INFO]: \tDebug build: "$BUILD_DEBUG"\n"

echo -n "[INFO]: Generating "$CONFIG_FILE"..."

echo -n "" > $CONFIG_FILE
echo "export OIP_BUILD_VERSION='\""$BUILD_VERSION"\"' " >> $CONFIG_FILE
echo "export OIP_BUILD_DATE='\""$BUILD_DATE"\"' " >> $CONFIG_FILE
echo "export OIP_BUILD_ABI="$BUILD_ABI >> $CONFIG_FILE
echo "export OIP_BUILD_DEBUG="$BUILD_DEBUG >> $CONFIG_FILE

echo " Done."
