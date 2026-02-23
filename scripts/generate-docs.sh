#!/bin/bash
#
# generate-docs.sh - Generate Doxygen documentation
# Usage: ./scripts/generate-docs.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Check if doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen not found. Please install it."
    exit 1
fi

echo "Generating documentation with Doxygen..."
cd "$PROJECT_DIR"
doxygen Doxyfile

echo "Documentation generated in docs/api/html/"
echo "Open docs/api/html/index.html in a browser to view."
