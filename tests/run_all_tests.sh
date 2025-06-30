#!/bin/bash

# Test Runner Script for 2D Truss Analysis
# Executes all available unit tests

# set -e  # Don't exit on test failures, handle them individually

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}===============================================${NC}"
echo -e "${BLUE}  2D Truss Analysis - Unit Test Suite${NC}"
echo -e "${BLUE}===============================================${NC}"

# Change to unit tests directory
cd "$(dirname "$0")/unit"

# List of test executables
TESTS=(
    "test_integration"
    "test_member"
    "test_minimal"
    "test_debug"
    "test_free"
    "test_manual"
    "test_pointer"
    "test_simple_integration"
    "test_working"
)

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

echo -e "\n${YELLOW}Discovering available tests...${NC}"

# Check which tests exist and are executable
AVAILABLE_TESTS=()
for test in "${TESTS[@]}"; do
    if [[ -x "$test" ]]; then
        AVAILABLE_TESTS+=("$test")
        echo -e "  ✅ Found: $test"
    else
        echo -e "  ⚠️  Missing: $test"
    fi
done

echo -e "\n${BLUE}Running ${#AVAILABLE_TESTS[@]} available tests...${NC}\n"

# Run each available test
for test in "${AVAILABLE_TESTS[@]}"; do
    echo -e "${YELLOW}Running $test...${NC}"
    
    if ./"$test"; then
        echo -e "${GREEN}✅ $test PASSED${NC}\n"
        ((PASSED_TESTS++))
    else
        echo -e "${RED}❌ $test FAILED${NC}\n"
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
done

# Summary
echo -e "${BLUE}===============================================${NC}"
echo -e "${BLUE}  TEST EXECUTION SUMMARY${NC}"
echo -e "${BLUE}===============================================${NC}"
echo -e "Total Tests: $TOTAL_TESTS"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"

if [[ $FAILED_TESTS -gt 0 ]]; then
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
    echo -e "\n${RED}⚠️  Some tests failed. Check the output above for details.${NC}"
    exit 1
else
    echo -e "\n${GREEN}🎉 All tests passed successfully! 🎉${NC}"
    exit 0
fi
