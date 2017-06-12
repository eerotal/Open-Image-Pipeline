## Open Image Pipeline - A plugin based image processing program written in C

Open Image Pipeline or OIP for short is a plugin based highly modular
image processing program written entirely in C. OIP is designed around
the concept of using plugins for image manipulation; no actual image
manipulation code is included in the main OIP binary. The main objectives
of Open Image Pipeline are to be fast and modular. Speed is achieved by
using C as the programming language and by keeping optimization in mind
while programming. Modularity on the other hand is achieved by the
use of plugins.

**NOTE: This program is still under VERY heavy development. It probably
doesn't work correctly yet and is full of bugs etc. Do not use it for anything
important. The most stable branch right now is master, which I try to keep
compilable. However, I can't guarantee the master branch will work correctly
either.**

### How to use?

Open Image Pipeline currently only has a text based command line interface,
however, a GUI based interface will probably be implemented later. The only
operating system where Open Image Pipeline currently works is Linux and even
there it requires quite a bit of computer knowledge to be able to run the
program. If you still want to give it a try, check out the *Compiling* section
below.


## Plugins

Below is a list of links to the plugins I have programmed for OIP. Currently
the only way to use them is to compile them yourself and place them in the
`plugins` directory of OIP (or use the `install` target of the plugin's makefile).

- <https://github.com/eerotal/OIP-Curves-plugin>
- <https://github.com/eerotal/OIP-Convolution-Matrix-plugin>

### Dependencies

FreeImage 3

### Compiling

Open Image Pipeline currently only works on Linux. The steps to compiling
OIP are listed below.

1. Install FreeImage and the FreeImage development files. On Debian
Jessie this can be accomplished by running
`sudo apt-get install libfreeimage3 libfreeimage-dev`.

2. Compile the Open Image Pipeline Core by running `make oipcore`.

3. [Optional] Compile the Open Image Pipeline submodules by running
`make oipmodules`. These modules are only needed if you intend to
develop and compile plugins.

4. [Optional] Compile the Open Image Pipeline Shell by running
`make oipshell`. This part is technically optional, however, the
OIP Shell is currently the only way to use Open Image Pipeline apart
from directly using it from the C language. The compiled binary is
placed into the folder `src/oipshell/bin/`.

5. Obtain source code for plugins and compile them yourself or obtain
the plugin binaries directly. For now you most likely need to compile
the plugins yourself, so remember to follow the instructions in step 3.
first and then follow the compilation instructions of the plugins.

6. Place the plugin libraries into the `plugins` folder in the main
Open Image Pipeline directory.

8. Add the path to the OIP Core shared library to your library search
path by running `export LD_LIBRARY_PATH=<PATH_TO_OIP_DIR>/src/oipcore/oipcore/bin`.
You must run this after every reboot if you want to use OIP since
the setting resets every time the computer is restarted. Alternatively
you can try to find a more permanent way of adding the path to the
library search path yourself.

7. Now you're ready to use Open Image Pipeline by using the OIP Shell
or by calling functions from the C language.  

You can also compile every part of Open Image Pipeline by just running
`make all`. By passing `DEBUG=1` with the make command you can enable
debug information genration and a memory address sanitizer while compiling.
Note that you won't be able to use non-debug builds of plugins with a
debug build of Open Image Pipeline and vice versa.  

The makefile has the following additional targets

#### clean-all

Delete all the files created by the compilation including the files
created when compiling the modules.

#### LOC

Count the total lines of code in this project.


## Open Source Libraries used

### FreeImage

This software uses the FreeImage open source image library.
See http://freeimage.sourceforge.net for details.
FreeImage is used under the GNU GPL, version 3.

You can find the full license in the file FreeImage-License-GNU-GPLv3.txt
in the source root of Open Image Pipeline.

## License

Copyright 2017 Eero Talus

This file is part of Open Image Pipeline.

Open Image Pipeline is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Open Image Pipeline is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Open Image Pipeline.  If not, see <http://www.gnu.org/licenses/>.
