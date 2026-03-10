#!/bin/bash
#
# run-static-analysis.sh - Run static analysis checks
# Usage: ./scripts/run-static-analysis.sh
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "Running static analysis checks..."

# Check for cppcheck
if command -v cppcheck &> /dev/null; then
    echo "Running cppcheck..."
    cppcheck --enable=all --suppress=missingIncludeSystem \
        "$PROJECT_DIR"/src \
        --error-exitcode=0
else
    echo "Warning: cppcheck not installed. Skipping cppcheck."
fi

# Check for clang-tidy
if command -v clang-tidy &> /dev/null; then
    echo "Running clang-tidy..."
    # This requires compile_commands.json from build directory
    if [ -f "$PROJECT_DIR/build/compile_commands.json" ]; then
        find "$PROJECT_DIR"/src -name "*.cpp" | head -10 | while read file; do
            echo "  Analyzing $file..."
            clang-tidy -p "$PROJECT_DIR/build" "$file" || true
        done
    else
        echo "Warning: compile_commands.json not found. Run ./scripts/build.sh first."
    fi
else
    echo "Warning: clang-tidy not installed. Skipping clang-tidy."
fi

echo "Static analysis complete!"
