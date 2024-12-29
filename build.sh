#!/bin/sh

clear
mkdir -p build
cmake . -B build -DCMAKE_BUILD_TYPE=RELWITHDEBINFO
cmake --build build
