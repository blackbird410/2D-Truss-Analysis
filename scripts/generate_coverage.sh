#!/bin/bash
# Coverage Report Generation Script
# 2D Truss Analysis C++ - v3.0.0
# Automated coverage analysis using gcov/lcov

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Configuration
BUILD_DIR="${PROJECT_DIR}/build"
COVERAGE_DIR="${BUILD_DIR}/coverage"
REPORT_DIR="${PROJECT_DIR}/docs/testing/coverage-reports"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  2D Truss Analysis - Coverage Report Generator${NC}"
echo -e "${BLUE}================================================${NC}"
echo ""

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"
command -v gcov >/dev/null 2>&1 || { echo -e "${RED}Error: gcov not found. Install gcc.${NC}"; exit 1; }
command -v lcov >/dev/null 2>&1 || { echo -e "${RED}Error: lcov not found. Install lcov.${NC}"; exit 1; }
command -v genhtml >/dev/null 2>&1 || { echo -e "${RED}Error: genhtml not found. Install lcov.${NC}"; exit 1; }
command -v bc >/dev/null 2>&1 || { echo -e "${RED}Error: bc not found. Install bc.${NC}"; exit 1; }
echo -e "${GREEN}✓ All prerequisites satisfied${NC}"
echo ""

# Clean previous coverage data
echo -e "${YELLOW}Cleaning previous coverage data...${NC}"
rm -rf "$COVERAGE_DIR"
mkdir -p "$COVERAGE_DIR"
mkdir -p "$REPORT_DIR"
find "$BUILD_DIR" -name "*.gcda" -delete
echo -e "${GREEN}✓ Cleanup complete${NC}"
echo ""

# Rebuild with coverage flags
echo -e "${YELLOW}Rebuilding project with coverage instrumentation...${NC}"
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON
cmake --build "$BUILD_DIR" --target clean
cmake --build "$BUILD_DIR" --target all_gtest_tests --parallel
echo -e "${GREEN}✓ Build complete${NC}"
echo ""

# Run all registered test suites via ctest.
# ctest honours each test's WORKING_DIRECTORY and ENVIRONMENT properties
# (e.g. QT_QPA_PLATFORM=offscreen for widget and GUI integration tests),
# and automatically includes unit_tests_gui_widgets and gui_integration_test
# when BUILD_GUI=ON — tests that were previously missing from this script.
echo -e "${YELLOW}Running all test suites via ctest...${NC}"
ctest --test-dir "$BUILD_DIR" --output-on-failure && CTEST_RESULT=0 || CTEST_RESULT=$?
echo -e "${GREEN}✓ Test suites executed${NC}"
echo ""

# Capture coverage data
echo -e "${YELLOW}Capturing coverage data...${NC}"
lcov --capture --directory "$BUILD_DIR" --output-file "$COVERAGE_DIR/coverage.info" \
     --rc branch_coverage=1 \
     --ignore-errors format,inconsistent,mismatch --quiet

# Filter out system headers, Homebrew-installed Qt framework headers (macOS),
# and test files.  The /opt/homebrew/* pattern removes Qt*.framework/Headers/
# paths that lcov picks up on macOS but which are not project code.
lcov --remove "$COVERAGE_DIR/coverage.info" \
     '/usr/*' \
     '/opt/homebrew/*' \
     '*/build/*' \
     '*/tests/*' \
     '*/include/gtest/*' \
     '*/include/gmock/*' \
     '*/include/eigen3/*' \
     --output-file "$COVERAGE_DIR/coverage_filtered.info" \
     --rc branch_coverage=1 --ignore-errors format,inconsistent,unused,mismatch --quiet

echo -e "${GREEN}✓ Coverage data captured${NC}"
echo ""

# Generate HTML report
echo -e "${YELLOW}Generating HTML coverage report...${NC}"
genhtml "$COVERAGE_DIR/coverage_filtered.info" \
        --output-directory "$COVERAGE_DIR/html" \
        --title "2D Truss Analysis Coverage Report" \
        --legend \
        --show-details \
        --branch-coverage \
        --rc derive_function_end_line=1 \
        --ignore-errors inconsistent,category,mismatch \
        --quiet

echo -e "${GREEN}✓ HTML report generated${NC}"
echo ""

# Generate text summary  
echo -e "${YELLOW}Generating coverage summary...${NC}"
lcov --summary "$COVERAGE_DIR/coverage_filtered.info" \
     --rc branch_coverage=1 --ignore-errors format,inconsistent,mismatch > "$COVERAGE_DIR/summary.txt" 2>&1

# Extract key metrics using more specific patterns
# Pattern explanation: Match a colon followed by space, then capture number.number, then %
# Using [^%] to explicitly avoid matching the % in the replacement part incorrectly
LINE_COVERAGE=$(grep -E "lines" "$COVERAGE_DIR/summary.txt" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%$//')
FUNCTION_COVERAGE=$(grep -E "functions" "$COVERAGE_DIR/summary.txt" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%$//')
BRANCH_COVERAGE=$(grep -E "branches" "$COVERAGE_DIR/summary.txt" | grep -oE '[0-9]+\.[0-9]+%' | head -1 | sed 's/%$//')

# Create timestamped summary
cat > "$REPORT_DIR/coverage_${TIMESTAMP}.txt" <<EOF
================================================================================
2D Truss Analysis - Coverage Report
Generated: $(date)
================================================================================

Summary:
--------
Line Coverage:     ${LINE_COVERAGE}%
Function Coverage: ${FUNCTION_COVERAGE}%
Branch Coverage:   ${BRANCH_COVERAGE}%

Test Results:
-------------
All Tests (ctest): $([ ${CTEST_RESULT} -eq 0 ] && echo "PASSED" || echo "FAILED")

Detailed Report:
----------------
HTML: file://$COVERAGE_DIR/html/index.html
Raw:  $COVERAGE_DIR/coverage_filtered.info

EOF

cat "$COVERAGE_DIR/summary.txt" >> "$REPORT_DIR/coverage_${TIMESTAMP}.txt"

# Create latest symlink
ln -sf "coverage_${TIMESTAMP}.txt" "$REPORT_DIR/coverage_latest.txt"

# Display results
echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  Coverage Report Summary${NC}"
echo -e "${BLUE}================================================${NC}"
echo ""
echo -e "${GREEN}Line Coverage:     ${LINE_COVERAGE}%${NC}"
echo -e "${GREEN}Function Coverage: ${FUNCTION_COVERAGE}%${NC}"
echo -e "${GREEN}Branch Coverage:   ${BRANCH_COVERAGE}%${NC}"
echo ""
echo -e "${BLUE}HTML Report:${NC} file://$COVERAGE_DIR/html/index.html"
echo -e "${BLUE}Text Summary:${NC} $REPORT_DIR/coverage_${TIMESTAMP}.txt"
echo ""

# Check coverage targets
if (( $(echo "$LINE_COVERAGE >= 72" | bc -l) )); then
    echo -e "${GREEN}✓ Line coverage target met (72%+)${NC}"
else
    echo -e "${YELLOW}⚠ Line coverage below target: ${LINE_COVERAGE}% < 72%${NC}"
fi

echo ""
echo -e "${GREEN}Coverage generation complete!${NC}"
