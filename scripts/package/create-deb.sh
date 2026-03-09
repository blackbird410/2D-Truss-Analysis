#!/bin/bash
#
# create-deb.sh - Create Debian packages for 2D Truss Analysis
#
# Usage:
#   ./scripts/package/create-deb.sh [version]
#
# The script configures a fresh build with CMAKE_INSTALL_PREFIX=/usr,
# builds the project, then invokes CPack to produce per-component .deb files:
#
#   2d-truss-analysis-common_<ver>_amd64.deb   shared data, config, examples
#   2d-truss-analysis-cli_<ver>_amd64.deb       CLI executable
#   2d-truss-analysis_<ver>_amd64.deb           GUI executable + desktop assets
#
# Prerequisites (Ubuntu/Debian):
#   sudo apt-get install cmake ninja-build build-essential dpkg-dev \
#       qt6-base-dev qt6-svg-dev libqt6svgwidgets6
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-package"
VERSION="${1:-$(cmake -P "${PROJECT_DIR}/cmake/PrintVersion.cmake" 2>/dev/null || echo "3.0.0")}"

echo "=================================================="
echo " 2D Truss Analysis — Debian Package Builder"
echo " Version : ${VERSION}"
echo " Build   : ${BUILD_DIR}"
echo "=================================================="

# ---------------------------------------------------------------------------
# vcpkg toolchain detection
# ---------------------------------------------------------------------------
VCPKG_TOOLCHAIN_ARG=""
if [[ -f "${PROJECT_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" ]]; then
    VCPKG_TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=${PROJECT_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "vcpkg  : submodule at ${PROJECT_DIR}/vcpkg"
elif [[ -n "${VCPKG_ROOT:-}" && -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]]; then
    VCPKG_TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    echo "vcpkg  : VCPKG_ROOT at ${VCPKG_ROOT}"
else
    echo "vcpkg  : not found; relying on system packages for Eigen3 / nlohmann-json / tinyxml2"
fi

# Verify that dpkg-shlibdeps is available (needed by CPack DEB)
if ! command -v dpkg-shlibdeps &>/dev/null; then
    echo "ERROR: dpkg-shlibdeps not found. Install it with:"
    echo "  sudo apt-get install dpkg-dev"
    exit 1
fi

# ---------------------------------------------------------------------------
# Configure
# ---------------------------------------------------------------------------
echo ""
echo "[1/3] Configuring..."
rm -rf "${BUILD_DIR}"
cmake -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DBUILD_TESTING=OFF \
    -DBUILD_GUI=ON \
    -DVCPKG_BUILD_TYPE=release \
    ${VCPKG_TOOLCHAIN_ARG}

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
echo "[2/3] Building..."
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

# ---------------------------------------------------------------------------
# Package
# ---------------------------------------------------------------------------
echo "[3/3] Packaging..."
cd "${BUILD_DIR}"
cpack -G DEB --verbose
cpack -G TGZ

echo ""
echo "Packages produced in ${BUILD_DIR}:"
ls -lh "${BUILD_DIR}"/2d-truss-analysis*.deb "${BUILD_DIR}"/2d-truss-analysis*.tar.gz 2>/dev/null || true
echo ""
echo "Install with:"
echo "  sudo dpkg -i ${BUILD_DIR}/2d-truss-analysis_${VERSION}_amd64.deb"
echo "  sudo apt-get install -f   # resolve any missing runtime dependencies"

