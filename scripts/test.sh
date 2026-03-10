#!/bin/bash
#
# test.sh - Test execution script for 2D Truss Analysis
# Usage: ./scripts/test.sh [test-type]
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

TEST_TYPE="${1:-all}"

# Ensure build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found. Run ./scripts/build.sh first."
    exit 1
fi

# Check if BUILD_TESTING is enabled
if grep -q "BUILD_TESTING:BOOL=OFF" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
    echo "ERROR: Tests are disabled (BUILD_TESTING=OFF)"
    echo "Please reconfigure with: ./scripts/build.sh --clean"
    echo "Or manually: cmake -B build -DBUILD_TESTING=ON"
    exit 1
fi

# Verify that test executables exist
if ! ctest --test-dir "$BUILD_DIR" -N &>/dev/null || [ "$(ctest --test-dir "$BUILD_DIR" -N 2>&1 | grep -c 'Test #')" -eq 0 ]; then
    echo "ERROR: No tests found in build directory"
    echo "This may indicate that BUILD_TESTING was OFF during configuration."
    echo "Please reconfigure with: ./scripts/build.sh --clean"
    exit 1
fi

case "$TEST_TYPE" in
    all)
        echo "Running all tests..."
        ctest --test-dir "$BUILD_DIR" --output-on-failure --parallel
        ;;
    unit)
        echo "Running unit tests..."
        ctest --test-dir "$BUILD_DIR" -R "unit_tests" --output-on-failure --parallel
        ;;
    integration)
        echo "Running integration tests..."
        ctest --test-dir "$BUILD_DIR" -R "integration_tests" --output-on-failure --parallel
        ;;
    gui)
        echo "Running GUI integration tests..."
        ctest --test-dir "$BUILD_DIR" -R "gui_integration_tests" --output-on-failure --parallel
        ;;
    *)
        echo "Unknown test type: $TEST_TYPE"
        echo "Valid options: all, unit, integration, gui"
        exit 1
        ;;
esac

echo "Tests complete!"
