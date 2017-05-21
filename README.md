## Open Image Pipeline - A plugin based image processing program written in C

Open Image Pipeline or OIP for short is a plugin based highly modular
image processing program written entirely in C. OIP is designed around
the concept of using plugins for image manipulation; no actual image
manipulation code is included in the main OIP binary. The actual hard work
is done by individual plugins that are loaded into OIP as shared libraries
from .so files. When OIP loads a plugin, it places information
about the plugin into a "pipeline" that is basically a sequence of the
different plugins that have been loaded. When OIP is instructed to process
an image, it feeds it into the pipeline where it goes through every
plugin one by one. After the last plugin has processed the image data, 
it is saved into a file. Basically OIP without plugins is absolutely
useless, since it can't do anything to the image.

The main objectives of Open Image Pipeline are to be fast, modular and
to have a simple API for plugins to interact with. Speed is achieved by
using C as the programming language and by keeping optimization in mind
while programming. Modularity is achieved by the use of plugins. The
API for plugins only has one structure that contains a few function
pointers that OIP needs and information about the plugin and it's author.

## Dependencies

  - FreeImage 3.15.4

## How to build?

1. Install FreeImage and the FreeImage development files. On Debian
Jessie this can be accomplished by running
`sudo apt-get install libfreeimage3 libfreeimage-dev`.

2. Run `make main`. If you intend to do plugin development run
`make modules` too. The resulting binary is put into the directory
`bin`. Plugins can be copied into the `plugins` directory.
