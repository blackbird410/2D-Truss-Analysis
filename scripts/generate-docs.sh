#!/bin/bash
#
# generate-docs.sh - Generate Doxygen documentation
# Usage: ./scripts/generate-docs.sh
#
# Note: This script uses the CMake build system to generate documentation.
#       The Doxyfile is generated from Doxyfile.in during CMake configuration.
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

# Check if cmake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found. Please install it."
    exit 1
fi

# Check if doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen not found. Please install it."
    exit 1
fi

echo "Generating documentation with Doxygen..."

# Configure CMake if build directory doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found. Configuring CMake..."
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_GUI=OFF
fi

# Check if Doxyfile was generated
if [ ! -f "$BUILD_DIR/Doxyfile" ]; then
    echo "Doxyfile not found. Reconfiguring CMake..."
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DBUILD_GUI=OFF
fi

# Build documentation target
cmake --build "$BUILD_DIR" --target docs

echo ""
echo "Documentation generated successfully!"
echo "  Location: $PROJECT_DIR/docs/api/html/index.html"
echo ""
echo "To view the documentation:"
echo "  open $PROJECT_DIR/docs/api/html/index.html"
