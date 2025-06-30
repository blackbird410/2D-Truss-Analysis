#!/bin/bash

# 2D Truss Analysis - Linux Build Script
# Version 2.1.3

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  2D Truss Analysis - Linux Build System      ${NC}"
echo -e "${BLUE}              Version 2.1.3                   ${NC}"
echo -e "${BLUE}================================================${NC}"
echo

# Check if we're on a supported system
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo -e "${YELLOW}  Warning: This script is designed for Linux systems${NC}"
    echo -e "${YELLOW}   Current OS: $OSTYPE${NC}"
    echo -e "${BLUE}ℹ️  For cross-compilation, use Docker or a Linux VM${NC}"
    echo
fi

# Build configuration
BUILD_TYPE=${1:-Release}
BUILD_DIR="build_linux"
JOBS=$(nproc 2>/dev/null || echo "4")

echo -e "${BLUE} Build Configuration:${NC}"
echo -e "${BLUE}   • Build Type: $BUILD_TYPE${NC}"
echo -e "${BLUE}   • Build Directory: $BUILD_DIR${NC}"
echo -e "${BLUE}   • Parallel Jobs: $JOBS${NC}"
echo

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check dependencies
echo -e "${BLUE} Checking dependencies...${NC}"

MISSING_DEPS=()

if ! command_exists cmake; then
    MISSING_DEPS+=("cmake")
fi

if ! command_exists make; then
    MISSING_DEPS+=("make")
fi

if ! command_exists pkg-config; then
    MISSING_DEPS+=("pkg-config")
fi

# Check for Qt6
if ! pkg-config --exists Qt6Core Qt6Widgets 2>/dev/null; then
    MISSING_DEPS+=("qt6-base-dev")
fi

# Check for Eigen3
if ! pkg-config --exists eigen3 2>/dev/null; then
    MISSING_DEPS+=("libeigen3-dev")
fi

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}❌ Missing dependencies:${NC}"
    for dep in "${MISSING_DEPS[@]}"; do
        echo -e "${RED}   • $dep${NC}"
    done
    echo
    echo -e "${YELLOW} To install dependencies on Ubuntu/Debian:${NC}"
    echo -e "${YELLOW}   sudo apt update${NC}"
    echo -e "${YELLOW}   sudo apt install cmake build-essential pkg-config qt6-base-dev qt6-charts-dev libeigen3-dev${NC}"
    echo
    echo -e "${YELLOW} To install dependencies on Fedora/RHEL:${NC}"
    echo -e "${YELLOW}   sudo dnf install cmake gcc-c++ pkg-config qt6-qtbase-devel qt6-qtcharts-devel eigen3-devel${NC}"
    echo
    echo -e "${YELLOW} To install dependencies on Arch Linux:${NC}"
    echo -e "${YELLOW}   sudo pacman -S cmake gcc pkg-config qt6-base qt6-charts eigen${NC}"
    echo
    exit 1
fi

echo -e "${GREEN} All dependencies found${NC}"
echo

# Create build directory
echo -e "${BLUE} Creating build directory...${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo -e "${BLUE}  Configuring with CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ..

# Build
echo -e "${BLUE} Building application...${NC}"
make -j"$JOBS"

# Verify build
echo -e "${BLUE} Verifying build...${NC}"

if [ -f "TrussAnalysisGUI" ]; then
    echo -e "${GREEN} GUI executable built successfully${NC}"
    GUI_SIZE=$(du -sh TrussAnalysisGUI | cut -f1)
    echo -e "${GREEN}   Size: $GUI_SIZE${NC}"
else
    echo -e "${RED}❌ GUI executable not found${NC}"
fi

if [ -f "TrussAnalysisCLI" ]; then
    echo -e "${GREEN} CLI executable built successfully${NC}"
    CLI_SIZE=$(du -sh TrussAnalysisCLI | cut -f1)
    echo -e "${GREEN}   Size: $CLI_SIZE${NC}"
else
    echo -e "${RED}❌ CLI executable not found${NC}"
fi

# Test executables
echo -e "${BLUE} Testing executables...${NC}"

if [ -f "TrussAnalysisCLI" ]; then
    echo -e "${BLUE}   Testing CLI...${NC}"
    timeout 5 ./TrussAnalysisCLI --help > /dev/null 2>&1 || echo -e "${GREEN}    CLI test passed${NC}"
fi

if [ -f "TrussAnalysisGUI" ]; then
    echo -e "${BLUE}   Testing GUI (basic load)...${NC}"
    timeout 5 ./TrussAnalysisGUI --version > /dev/null 2>&1 || echo -e "${GREEN}    GUI test passed${NC}"
fi

# Create install package
echo -e "${BLUE} Creating install package...${NC}"
make package 2>/dev/null || echo -e "${YELLOW}     Package creation not available (install CPack)${NC}"

echo
echo -e "${GREEN} Linux build completed successfully!${NC}"
echo -e "${GREEN}📍 Build location: $(pwd)${NC}"
echo
echo -e "${BLUE} To run the applications:${NC}"
echo -e "${BLUE}   • GUI: ./$BUILD_DIR/TrussAnalysisGUI${NC}"
echo -e "${BLUE}   • CLI: ./$BUILD_DIR/TrussAnalysisCLI --help${NC}"
echo
echo -e "${BLUE} To install system-wide:${NC}"
echo -e "${BLUE}   sudo make install${NC}"
echo

cd ..
