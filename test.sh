#!/bin/sh

./build.sh
cd build
gdb --args ctest "$@"
