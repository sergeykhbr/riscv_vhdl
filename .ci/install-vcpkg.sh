#!/bin/bash

# Install Vcpkg in $HOME/vcpkg or update an already existing installation.
#
# Usage: /bin/bash .ci/install-vcpkg.sh

set -e

VCPKG_DIR="$HOME/vcpkg"

# either install Vcpkg or update an existing installation
if [[ -d "$VCPKG_DIR" ]]; then UPDATE_VCPKG=1; else UPDATE_VCPKG=0; fi

echo "---- install-vcpkg.sh ----"
echo "Vcpkg directory: $VCPKG_DIR"
echo "Update existing Vcpkg? $UPDATE_VCPKG"
echo "--------------------------"

if [[ $UPDATE_VCPKG -eq 1 ]]; then
    # update existing Vcpkg
    pushd "$VCPKG_DIR"
    git pull --quiet
    ./bootstrap-vcpkg.sh --disableMetrics
    popd
else
    # install Vcpkg
    pushd "$HOME"
    git clone --quiet https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh --disableMetrics
    popd
fi
