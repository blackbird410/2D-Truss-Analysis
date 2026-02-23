# Generate Golden Masters Script

This utility generates corrected golden master files for integration testing.

## Building

To build this utility:

```bash
cd /Users/neil/dev/repos/2D-Truss-Analysis-cpp
g++ -std=c++20 -I./include -I./src scripts/generate_golden_masters.cpp -o scripts/generate_golden_masters
```

## Usage

After building, run the utility to regenerate golden master test files.
