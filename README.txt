Nuklei http://nuklei.sf.net
---------------------------

For detailed install docs, see

http://nuklei.sourceforge.net/doxygen/group__install.html

Summarized install instructions are provided below.

Dependencies:
-------------

sudo apt-get install gcc g++ make pkg-config python libgsl-dev libblas-dev liblapack-dev libboost-all-dev libpcl-dev libcgal-dev

Quick install:
--------------

Nuklei's build system is SCons. Install with

./scons.py install

This directory also contains a Makefile and a CMakeLists.txt, which both trigger SCons.
Read the Makefile to see what shortcuts it defines. The CMakeLists.txt is useless at this
point.

Usage:
------

Nuklei installs a pkg-config nuklei.pc file, and a CMake nuklei-config.cmake file. See
INSTALL_PREFIX/lib/{pkgconfig,cmake}.
