#!/bin/bash
# edit CROSS_COMPILE to match your toolchain prefix/path
export CROSS_COMPILE=arm-linux-gnueabihf-
export PATH=/opt/gcc-arm/bin:$PATH
export SYSROOT=/opt/gcc-arm/arm-linux-gnueabihf/sysroot
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
echo "Loaded toolchain env: CROSS_COMPILE=${CROSS_COMPILE}"
