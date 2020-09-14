Nuklei http://renaud-detry.net/nuklei
-------------------------------------

For detailed install docs, see

http://renaud-detry.net/nuklei/group__install.html

Summarized install instructions are provided below.

Dependencies:
-------------

sudo apt-get install gcc g++ make pkg-config python libgsl-dev libblas-dev liblapack-dev libboost-all-dev libpcl-dev libcgal-dev

Quick install:
--------------

Nuklei's build system is SCons. Install with

./scons.py install

Usage:
------

Nuklei installs a pkg-config nuklei.pc file, and a CMake nuklei-config.cmake file. See
INSTALL_PREFIX/lib/{pkgconfig,cmake}.
