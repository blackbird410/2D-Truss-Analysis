#!/bin/bash
#
# install-deps-ubuntu.sh - Install dependencies on Ubuntu/Debian
# Usage: ./scripts/install-deps-ubuntu.sh
#

set -e

echo "Installing dependencies for Ubuntu/Debian..."

# Update package list
sudo apt-get update

# Install build tools
echo "Installing build tools..."
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git

# Install development libraries
echo "Installing development libraries..."
sudo apt-get install -y \
    libeigen3-dev \
    nlohmann-json3-dev \
    libtinyxml2-dev \
    qt6-base-dev \
    libqt6core6 \
    libqt6widgets6 \
    qt6-svg-dev \
    libqt6svg6 \
    libqt6svgwidgets6

# Install optional tools
echo "Installing optional tools..."
sudo apt-get install -y \
    clang-format \
    clang-tidy \
    cppcheck \
    doxygen \
    lcov \
    google-test \
    libgtest-dev

echo "Dependencies installed!"
