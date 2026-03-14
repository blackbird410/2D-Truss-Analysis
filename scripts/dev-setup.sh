#!/bin/bash
# =============================================================================
# Development Container Setup Script
# Quick configuration and build for the development environment
# =============================================================================

set -e  # Exit on error

INSTALL_TOOLS="false"
if [ "${1:-}" = "setup" ] || [ "${1:-}" = "--install-tools" ]; then
    INSTALL_TOOLS="true"
    shift || true
fi

install_tools() {
    if ! command -v apt-get >/dev/null 2>&1; then
        echo "Skipping tool installation (apt-get not available)."
        return 0
    fi

    if [ "$(id -u)" -ne 0 ]; then
        if command -v sudo >/dev/null 2>&1; then
            SUDO="sudo"
        else
            echo "Skipping tool installation (requires root or sudo)."
            return 0
        fi
    else
        SUDO=""
    fi

    echo "Installing development tools..."
    ${SUDO} apt-get update
    ${SUDO} apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        git \
        clang \
        clang-tidy \
        clang-format \
        cppcheck \
        gdb \
        valgrind \
        lcov \
        gcovr \
        libeigen3-dev \
        nlohmann-json3-dev \
        libtinyxml2-dev \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-charts-dev \
        libqt6charts6-dev \
        libqt6core6 \
        libqt6widgets6
}

echo "========================================"
echo "  2D Truss Analysis - Dev Setup"
echo "========================================"
echo ""

# Install tools if requested
if [ "$INSTALL_TOOLS" = "true" ]; then
    install_tools
fi

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Are you in /workspace?"
    exit 1
fi

# Remove stale build directory only if there's a path mismatch or running in Docker
# This prevents CMakeCache.txt issues when Docker mounts change the workspace path
if [ -d "build" ] && [ -f "build/CMakeCache.txt" ]; then
    # Check if we're in Docker (/.dockerenv exists) or have DOCKER env var
    IN_DOCKER=false
    if [ -f "/.dockerenv" ] || [ -n "${DOCKER:-}" ]; then
        IN_DOCKER=true
    fi
    
    # Extract CMakeCache source dir and compare with current directory
    CACHE_SOURCE_DIR=$(grep "CMAKE_HOME_DIRECTORY:" build/CMakeCache.txt 2>/dev/null | cut -d= -f2 || echo "")
    CURRENT_DIR=$(pwd)
    
    if [ "$IN_DOCKER" = "true" ] || [ "$CACHE_SOURCE_DIR" != "$CURRENT_DIR" ]; then
        echo "Removing stale build directory (path mismatch or Docker environment)..."
        rm -rf build
    fi
fi

# Ask user for build type (non-interactive friendly)
echo "Select build configuration:"
echo "  1) CLI-only (faster, no Qt6) [DEFAULT]"
echo "  2) Full build with GUI (requires Qt6)"
echo ""

choice="${SETUP_CHOICE:-}"
if [ -z "$choice" ]; then
    if [ -t 0 ]; then
        read -p "Enter choice [1-2] (default: 1): " choice
    else
        choice="1"
    fi
fi
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
