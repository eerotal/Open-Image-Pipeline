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

## Developer corner

So, as you already may know, all the actual hard image processing work in OIP
is done by individual plugins which are loaded into OIP as shared libraries
from .so files. When OIP loads a plugin, it places information about the
plugin into a "pipeline" that is basically a sequence of the different
plugins that have been loaded. The plugins can also accept arguments that
tell the plugin what it needs to do to the image, however, these arguments
are plugin specific.  

Before an image can be fed into the pipeline, a JOB instance needs to be created.
The JOB type is a container that holds basic information about the
image to be processed. Additionally, because JOB instances can be fed to 
the pipeline multiple times if needed, the JOB instance holds information 
about the previous times it was fed into the pipeline. The information
the JOB instance holds about previous image processing passes
makes a process called plugin skipping possible. Plugin skipping happens
when OIP detects that the output of certain plugins has already been 
generated for the supplied image and it's stored in cache files. If this
happens, OIP skips the plugins in question and just feeds the cached
versions of the image to the plugins that don't have up-to-date data
in their cache files.

After the pipeline has processed the image contained in the JOB instance,
it puts it into a destination image buffer that's also located in
the JOB instance. The image can then be saved using job_save() and passing
the JOB instance and the requested filepath as the two arguments.

### Dependencies

FreeImage 3

### Compiling

Open Image Pipeline currently only works on Linux. The steps to compiling
OIP are listed below.

1. Install FreeImage and the FreeImage development files. On Debian
Jessie this can be accomplished by running
`sudo apt-get install libfreeimage3 libfreeimage-dev`.

2. Run `make main`. If you intend to do plugin development run
`make modules` too. The resulting binary is put into the directory
`bin`. Plugins can be copied into the `plugins` directory.  

The makefile has the following additional targets

#### clean

Delete the files created by the compilation of the main OIP binary.

#### clean-all

Delete all the files created by the compilation including the files
created when compiling the modules.

#### clean-modules

Delete the files created by the module compilation.

#### LOC

Count the total lines of code in this project.


## Open Source Libraries used

### Freeimage

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
