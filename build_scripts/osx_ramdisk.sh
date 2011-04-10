#!/bin/bash

if [ "$1" = "-u" ]; then
  tar -czf scons_build.tgz -C /vlt scons.build
  exit 0
fi

if [ ! -e /vlt/ ]; then
  diskutil erasevolume hfsx "ramdisk" `hdiutil attach -nomount ram://4165430`
fi

if [ ! -e /vlt/scons.build/ ]; then
  tar -f scons_build.tgz -C /vlt -xz
fi

./scons.py -j 3 install
