#!/bin/bash

# Build script for 2D Truss Analysis DMG creation
# Version 2.0.0

set -e  # Exit on any error

echo "===================================================="
echo "Building 2D Truss Analysis v2.0.0 for Distribution"
echo "===================================================="

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build_release
rm -rf *.dmg

# Create build directory
mkdir -p build_release
cd build_release

# Configure with Release settings
echo "Configuring build..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
      -DCMAKE_INSTALL_PREFIX=./install \
      ..

# Build the project
echo "Building project..."
make -j$(sysctl -n hw.ncpu)

# Test the build
echo "Testing the build..."
./TrussAnalysis.app/Contents/MacOS/TrussAnalysis -e

# Create the DMG package
echo "Creating DMG package..."
make install

# Use CPack to create the DMG
cpack

echo "Build completed successfully!"
echo "DMG file created in build_release directory"

# Move DMG to project root
mv *.dmg ../ 2>/dev/null || echo "No DMG files to move"

echo "Build process complete!"
