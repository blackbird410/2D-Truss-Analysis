#!/bin/bash
#
# format-code.sh - Format C++ code using clang-format
# Usage: ./scripts/format-code.sh [options]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found. Please install it."
    exit 1
fi

# Format source files
echo "Formatting C++ source files..."
find "$PROJECT_DIR"/src -type f \( -name "*.cpp" -o -name "*.hpp" \) | while read file; do
    echo "  Formatting $file..."
    clang-format -i "$file"
done

# Format test files
echo "Formatting test files..."
find "$PROJECT_DIR"/tests -type f -name "*.cpp" | while read file; do
    echo "  Formatting $file..."
    clang-format -i "$file"
done

echo "Code formatting complete!"
