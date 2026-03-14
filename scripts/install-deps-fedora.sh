#!/bin/bash
#
# install-deps-fedora.sh - Install dependencies on Fedora/RHEL
# Usage: ./scripts/install-deps-fedora.sh
#

set -e

echo "Installing dependencies for Fedora/RHEL..."

# Install build tools
echo "Installing build tools..."
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    git

# Install development libraries
echo "Installing development libraries..."
sudo dnf install -y \
    eigen3-devel \
    nlohmann-json-devel \
    tinyxml2-devel \
    qt6-qtbase-devel \
    qt6-qtsvg-devel

# Install optional tools
echo "Installing optional tools..."
sudo dnf install -y \
    clang-tools-extra \
    cppcheck \
    doxygen \
    lcov \
    gtest-devel

echo "Dependencies installed!"
