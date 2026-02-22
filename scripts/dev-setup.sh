#!/bin/bash
# =============================================================================
# Development Container Setup Script
# Quick configuration and build for the development environment
# =============================================================================

set -e  # Exit on error

echo "========================================"
echo "  2D Truss Analysis - Dev Setup"
echo "========================================"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Are you in /workspace?"
    exit 1
fi

# Ask user for build type
echo "Select build configuration:"
echo "  1) CLI-only (faster, no Qt6) [DEFAULT]"
echo "  2) Full build with GUI (requires Qt6)"
echo ""
read -p "Enter choice [1-2] (default: 1): " choice
choice=${choice:-1}

BUILD_GUI="OFF"
BUILD_TESTING="ON"
if [ "$choice" = "2" ]; then
    BUILD_GUI="ON"
    echo "Building with GUI support..."
else
    echo "Building CLI-only (faster)..."
    # Tests remain enabled for CLI-only builds; Qt6-dependent tests are
    # conditionally handled by CMake when BUILD_GUI=ON.
fi

# Configure build
echo ""
echo "Configuring CMake..."
cmake -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_GUI=$BUILD_GUI \
    -DBUILD_TESTING=$BUILD_TESTING \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DENABLE_COVERAGE=OFF

# Build
echo ""
echo "Building project..."
cmake --build build -j$(nproc)

# Summary
echo ""
echo "========================================"
echo "  ✓ Build Complete!"
echo "========================================"
echo ""
echo "Quick commands:"
echo "  ./build/TrussAnalysisCLI --help     # Show help"
echo "  ./build/TrussAnalysisCLI example    # Run example"
echo "  cd build && ctest                   # Run tests"
echo "  make test                           # Run tests (via Make)"
echo ""
echo "Rebuild after changes:"
echo "  cmake --build build -j\$(nproc)"
echo "  # or: make build"
echo ""
