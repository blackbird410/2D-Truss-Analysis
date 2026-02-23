#!/bin/bash
#
# build.sh - Build script wrapper for 2D Truss Analysis
# Usage: ./scripts/build.sh [options]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

# Parse arguments
BUILD_TYPE="Release"
PARALLEL_JOBS=$(nproc)
CLEAN_BUILD=false

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
        -j|--jobs)
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
echo "Configuring CMake (${BUILD_TYPE})..."
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

# Build
echo "Building (${PARALLEL_JOBS} jobs)..."
cmake --build . -j"$PARALLEL_JOBS"

echo "Build complete!"
