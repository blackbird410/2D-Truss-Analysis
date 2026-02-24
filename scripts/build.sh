#!/bin/bash
#
# build.sh - Build script wrapper for 2D Truss Analysis
# Usage: ./scripts/build.sh [options]
#
# Options:
#   --debug         Build in Debug mode (default: Release)
#   --release       Build in Release mode
#   --clean         Clean build directory before building
#   --no-tests      Disable test building (BUILD_TESTING=OFF)
#   -j <N>          Use N parallel jobs (default: number of CPU cores)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

# Parse arguments
BUILD_TYPE="Release"
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
CLEAN_BUILD=false
BUILD_TESTING="ON"

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --no-tests)
            BUILD_TESTING="OFF"
            shift
            ;;
        -j*)
            # Handle -j8 format (no space)
            PARALLEL_JOBS="${1#-j}"
            shift
            ;;
        -j|--jobs)
            # Handle -j 8 format (with space)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Clean if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"/*
fi

# Configure
echo "Configuring CMake (${BUILD_TYPE}, BUILD_TESTING=${BUILD_TESTING})..."
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DBUILD_TESTING="$BUILD_TESTING" \
      ..

# Build
echo "Building (${PARALLEL_JOBS} jobs)..."
cmake --build . -j"$PARALLEL_JOBS"

echo "Build complete!"
