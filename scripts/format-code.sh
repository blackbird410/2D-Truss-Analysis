#!/bin/bash
#
# format-code.sh - Format C++ code using clang-format
# Usage: ./scripts/format-code.sh [options]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

MODE="apply"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --check)
            MODE="check"
            shift
            ;;
        --apply)
            MODE="apply"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--check|--apply]"
            exit 1
            ;;
    esac
done

if [ -z "${CLANG_FORMAT_BIN:-}" ]; then
    if command -v clang-format-14 &> /dev/null; then
        CLANG_FORMAT_BIN="clang-format-14"
    else
        CLANG_FORMAT_BIN="clang-format"
    fi
fi

if ! command -v "$CLANG_FORMAT_BIN" &> /dev/null; then
    echo "Error: $CLANG_FORMAT_BIN not found. Please install clang-format."
    exit 1
fi

echo "Using $CLANG_FORMAT_BIN ($($CLANG_FORMAT_BIN --version))"

FILES=$(find "$PROJECT_DIR"/src "$PROJECT_DIR"/include "$PROJECT_DIR"/tests \
    -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
    ! -path "*/vcpkg_installed/*" ! -path "*/build/*")

if [ -z "$FILES" ]; then
    echo "No files found to format."
    exit 0
fi

if [ "$MODE" = "check" ]; then
    echo "Checking C++ code formatting..."
    NEEDS_FORMAT=0

    for file in $FILES; do
        if ! "$CLANG_FORMAT_BIN" --dry-run --Werror "$file" 2>/dev/null; then
            echo "::error file=$file::File needs formatting"
            NEEDS_FORMAT=1
        fi
    done

    if [ $NEEDS_FORMAT -eq 1 ]; then
        echo ""
        echo "Error: Code formatting issues detected!"
        echo "Run 'make format' or './scripts/format-code.sh --apply' to fix formatting issues"
        exit 1
    fi

    echo "✓ All files are properly formatted"
    exit 0
fi

echo "Formatting C++ code..."
for file in $FILES; do
    echo "  Formatting $file..."
    "$CLANG_FORMAT_BIN" -i "$file"
done

echo "Code formatting complete!"
