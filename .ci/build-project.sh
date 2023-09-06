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

cd $(dirname "$0")

#if $1 is not set then use "build" as the default
BUILD_DIR=${1:-build}
VCPKG_DIR="$HOME/vcpkg"

# only pass toolchain file to CMake if Vcpkg is installed
if [[ -d "$VCPKG_DIR" ]]; then TOOLCHAIN="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"; else TOOLCHAIN=False; fi

echo "---- build-project.sh ----"
echo "BUILD_DIR: $BUILD_DIR"
echo "VCPKG_DIR: $VCPKG_DIR"
echo "CC: $CC"
echo "CXX: $CXX"
echo "CXXFLAGS: $CXXFLAGS"
echo "CMAKE_TOOLCHAIN_FILE: $TOOLCHAIN"
echo "--------------------------"

if [ -n "$CCACHE_DIR" ]; then
    mkdir -p "$CCACHE_DIR" && ./ci-ccache-maint.sh
fi

mkdir "$BUILD_DIR"
cmake -S ./debugger/cmake -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j
