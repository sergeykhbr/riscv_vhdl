#!/bin/bash

# Build project.
#
# The script assumes that it will be called from inside the project directory.
#
# Usage: /bin/bash .ci/build-project.sh [build-directory-name]
# - build-directory-name: Optional name of build directory. Default: build.
#
# Example 1: /bin/bash .ci/build-project.sh

set -e
set -x

# folder of current script
cd $(dirname "$0")
cd ../

#if $1 is not set then use "build" as the default
BUILD_DIR=${1:-build}

echo "---- build-project.sh ----"
echo "pwd: $PWD"
echo "BUILD_DIR: $BUILD_DIR"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXXFLAGS: $CXXFLAGS"
echo "CMAKE_TOOLCHAIN_FILE: $TOOLCHAIN"
echo "CCACHE_DIR: $CCACHE_DIR"
echo "SYSTEMC_SRC: $SYSTEMC_SRC"
echo "SYSTEMC_LIB: $SYSTEMC_LIB"
echo "--------------------------"

#if [ -n "$CCACHE_DIR" ]; then
#    mkdir -p "$CCACHE_DIR" && ./ci-ccache-maint.sh
#fi

mkdir "$BUILD_DIR"
cmake -S ./debugger/cmake -B "$BUILD_DIR"
cd $BUILD_DIR
make
