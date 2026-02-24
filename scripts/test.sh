#!/bin/bash
#
# test.sh - Test execution script for 2D Truss Analysis
# Usage: ./scripts/test.sh [test-type]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

TEST_TYPE="${1:-all}"

# Ensure build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found. Run ./scripts/build.sh first."
    exit 1
fi

cd "$BUILD_DIR"

# Check if BUILD_TESTING is enabled
if grep -q "BUILD_TESTING:BOOL=OFF" "$BUILD_DIR/CMakeCache.txt" 2>/dev/null; then
    echo "ERROR: Tests are disabled (BUILD_TESTING=OFF)"
    echo "Please reconfigure with: ./scripts/build.sh --clean"
    echo "Or manually: cmake -B build -DBUILD_TESTING=ON"
    exit 1
fi

case "$TEST_TYPE" in
    all)
        echo "Running all tests..."
        ctest --output-on-failure
        ;;
    unit)
        echo "Running unit tests..."
        ctest -R "UnitTests" --output-on-failure
        ;;
    integration)
        echo "Running integration tests..."
        ctest -R "IntegrationTests" --output-on-failure
        ;;
    gui)
        echo "Running GUI integration tests..."
        ctest -R "GUIIntegrationTests" --output-on-failure
        ;;
    *)
        echo "Unknown test type: $TEST_TYPE"
        echo "Valid options: all, unit, integration, gui"
        exit 1
        ;;
esac

echo "Tests complete!"
