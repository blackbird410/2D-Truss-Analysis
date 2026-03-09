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
#   --no-gui        Disable GUI (BUILD_GUI=OFF)
#   -j <N>          Use N parallel jobs (default: number of CPU cores)
#
# vcpkg:
#   The script automatically selects a CMake toolchain file in this order:
#     1. $PROJECT_DIR/vcpkg/scripts/buildsystems/vcpkg.cmake  (git submodule)
#     2. $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake         (env variable)
#   If neither is found, CMake finds dependencies via system package-manager
#   paths.  Install system Qt6 and other dev packages to build without vcpkg.
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
BUILD_GUI="ON"

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
        --no-gui)
            BUILD_GUI="OFF"
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

# ---------------------------------------------------------------------------
# vcpkg toolchain detection
# ---------------------------------------------------------------------------
VCPKG_TOOLCHAIN_ARG=""
if [[ -f "${PROJECT_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" ]]; then
    VCPKG_TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=${PROJECT_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "vcpkg: using submodule at ${PROJECT_DIR}/vcpkg"
elif [[ -n "${VCPKG_ROOT:-}" && -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]]; then
    VCPKG_TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    echo "vcpkg: using VCPKG_ROOT at ${VCPKG_ROOT}"
else
    echo "vcpkg: not found (submodule or VCPKG_ROOT). Falling back to system packages."
    echo "  To enable vcpkg: run 'git submodule update --init vcpkg' or set VCPKG_ROOT."
fi

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Clean if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"/*
fi

# Configure
echo "Configuring CMake (${BUILD_TYPE}, BUILD_TESTING=${BUILD_TESTING}, BUILD_GUI=${BUILD_GUI})..."
cd "$BUILD_DIR"
cmake \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTING="$BUILD_TESTING" \
    -DBUILD_GUI="$BUILD_GUI" \
    ${VCPKG_TOOLCHAIN_ARG} \
    ..

# Build
echo "Building (${PARALLEL_JOBS} jobs)..."
cmake --build . -j"$PARALLEL_JOBS"


echo "Build complete!"
