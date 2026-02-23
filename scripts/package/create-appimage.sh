#!/bin/bash
#
# create-appimage.sh - Create AppImage package
# Usage: ./scripts/package/create-appimage.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

echo "Creating AppImage package..."

# Check if linuxdeploy is installed
if ! command -v linuxdeploy &> /dev/null; then
    echo "Error: linuxdeploy not found. Please install it from https://github.com/linuxdeploy/linuxdeploy"
    exit 1
fi

# Ensure build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Run ./scripts/build.sh first."
    exit 1
fi

# Create AppImage
cd "$BUILD_DIR"
cmake --install . --prefix ../install
linuxdeploy-x86_64.AppImage --appdir=../install --output=appimage

echo "AppImage created"
