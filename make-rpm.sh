#!/bin/bash

# Assume this is repo root.
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

BUILD_DIR=$(mktemp -d)
PACKAGE_DIR=packages
INSTALL_PREFIX="/usr"
mkdir -p "$PACKAGE_DIR"
cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DUNTANGLE_TEST=Off -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX
cmake --build "$BUILD_DIR" -j$(nproc --ignore=2)
cpack -B "$PACKAGE_DIR" --config "$BUILD_DIR/CPackConfig.cmake"
rm -rf "$BUILD_DIR"