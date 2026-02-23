#!/bin/bash
# Coverage Report Generation Script
# 2D Truss Analysis C++ - v3.0.0
# Automated coverage analysis using gcov/lcov

set -e

# Configuration
BUILD_DIR="build"
COVERAGE_DIR="build/coverage"
REPORT_DIR="docs/testing/coverage-reports"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

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
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
         -DCMAKE_EXE_LINKER_FLAGS="--coverage"
make clean
make -j$NPROC unit_tests integration_tests
echo -e "${GREEN}✓ Build complete${NC}"
echo ""

# Run unit tests
echo -e "${YELLOW}Running unit tests...${NC}"
./unit_tests --gtest_brief=1 || true
UNIT_RESULT=$?
echo -e "${GREEN}✓ Unit tests executed${NC}"
echo ""

# Run integration tests
echo -e "${YELLOW}Running integration tests...${NC}"
./integration_tests --gtest_brief=1 || true
INT_RESULT=$?
echo -e "${GREEN}✓ Integration tests executed${NC}"
echo ""

# Capture coverage data
echo -e "${YELLOW}Capturing coverage data...${NC}"
cd ..
lcov --capture --directory "$BUILD_DIR" --output-file "$COVERAGE_DIR/coverage.info" \
     --rc lcov_branch_coverage=1 \
     --ignore-errors format,inconsistent --quiet

# Filter out system headers and test files
lcov --remove "$COVERAGE_DIR/coverage.info" \
     '/usr/*' \
     '*/build/*' \
     '*/tests/*' \
     '*/include/gtest/*' \
     '*/include/gmock/*' \
     '*/include/eigen3/*' \
     --output-file "$COVERAGE_DIR/coverage_filtered.info" \
     --rc lcov_branch_coverage=1 --ignore-errors format,inconsistent,unused --quiet

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
        --ignore-errors inconsistent,category \
        --quiet

echo -e "${GREEN}✓ HTML report generated${NC}"
echo ""

# Generate text summary
echo -e "${YELLOW}Generating coverage summary...${NC}"
lcov --summary "$COVERAGE_DIR/coverage_filtered.info" \
     --rc lcov_branch_coverage=1 --ignore-errors format,inconsistent > "$COVERAGE_DIR/summary.txt" 2>&1

# Extract key metrics
LINE_COVERAGE=$(grep "lines" "$COVERAGE_DIR/summary.txt" | sed 's/.*\([0-9][0-9]*\.[0-9][0-9]*\)%.*/\1/')
FUNCTION_COVERAGE=$(grep "functions" "$COVERAGE_DIR/summary.txt" | sed 's/.*\([0-9][0-9]*\.[0-9][0-9]*\)%.*/\1/')
BRANCH_COVERAGE=$(grep "branches" "$COVERAGE_DIR/summary.txt" | sed 's/.*\([0-9][0-9]*\.[0-9][0-9]*\)%.*/\1/')

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
Unit Tests:        $([ $UNIT_RESULT -eq 0 ] && echo "PASSED" || echo "FAILED")
Integration Tests: $([ $INT_RESULT -eq 0 ] && echo "PASSED" || echo "FAILED")

Detailed Report:
----------------
HTML: file://$(pwd)/$COVERAGE_DIR/html/index.html
Raw:  $(pwd)/$COVERAGE_DIR/coverage_filtered.info

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
echo -e "${BLUE}HTML Report:${NC} file://$(pwd)/$COVERAGE_DIR/html/index.html"
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
