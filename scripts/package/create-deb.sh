#!/bin/bash
#
# create-deb.sh - Create Debian package
# Usage: ./scripts/package/create-deb.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

echo "Creating Debian package..."

# Ensure build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Run ./scripts/build.sh first."
    exit 1
fi

# Create package
cd "$BUILD_DIR"
cpack -G DEB

echo "Debian package created in $BUILD_DIR"
