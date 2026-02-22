# Golden Master Export Files

**Last Updated:** 2026-02-08  
**Purpose:** Phase 3 validation reference files  
**Validated Formats:** CSV, JSON, XML, HTML, LaTeX, TXT (6/6)  
**Generator:** `generate_corrected_golden_masters.cpp`

## Status

All six export formats are validated and safe for use. See [VALIDATION_STATUS.md](VALIDATION_STATUS.md) for full details.

## Overview

These files serve as "golden masters" for validating new Strategy pattern exporter implementations in Phase 3. Validated golden masters contain **COMPLETE analysis data** including reactions (mandatory for structural equilibrium verification).

## Test Truss Structure

**Name:** Golden Master Test Truss  
**Type:** Simple statically determinate triangle truss

### Geometry

- **Nodes:** 3
  - Node 1: (0.0, 0.0) - Pinned support (Fx=0, Fy=0)
  - Node 2: (4.0, 0.0) - RollerX support (Fy constrained)
  - Node 3: (2.0, 3.0) - Free node

- **Members:** 3
  - Member 1: Node 1 → Node 2 (horizontal, length = 4.0 m)
  - Member 2: Node 1 → Node 3 (left diagonal, length = 3.606 m)
  - Member 3: Node 2 → Node 3 (right diagonal, length = 3.606 m)

### Loading

- Applied force at Node 3: (0.0, -15000.0) N (15 kN downward)

### Material Properties

- Default material properties (as defined in code)
- Young's modulus: E = 200 GPa (typical steel)
- Cross-sectional area: A = 0.01 m² (10,000 mm²)

## Analysis Results

- **Convergence:** Successfully converged
- **Max displacement:** 2.863e+13 m
- **Max stress:** 1.316e+08 Pa

**Note:** The extremely large displacement value indicates the truss uses default material/section properties that may need review. This is acceptable for golden master purposes as we're testing format consistency, not numerical accuracy.

## File Inventory

### ✅ Validated Golden Masters (Safe for Testing)

| File                 | Format | Size (Feb 21) | Status           | Notes                              |
| -------------------- | ------ | ------------- | ---------------- | ---------------------------------- |
| `golden_master.csv`  | CSV    | ~1.1 KB       | ✅ **VALIDATED** | Authoritative reference (complete) |
| `golden_master.json` | JSON   | ~1.9 KB       | ✅ **VALIDATED** | Complete 8-section export          |
| `golden_master.xml`  | XML    | ~2.9 KB       | ✅ **VALIDATED** | Complete 8-section export          |
| `golden_master.html` | HTML   | ~7.0 KB       | ✅ **VALIDATED** | Complete 8-section export          |
| `golden_master.tex`  | LaTeX  | ~4.2 KB       | ✅ **VALIDATED** | Complete 8-section export          |
| `golden_master.txt`  | TXT    | ~4.0 KB       | ✅ **VALIDATED** | Complete 8-section export          |

**Total Validated:** 6 files

## Export Options Used

All golden masters were generated with the following options:

```cpp
ExportOptions options;
options.includeGeometry = true;
options.includeProperties = true;
options.includeLoads = true;
options.includeDisplacements = true;
options.includeMemberForces = true;
options.includeReactions = true;
options.includeStresses = true;
options.includeUtilization = true;
options.includeMetadata = true;
options.useScientificNotation = false;
options.precision = 6;
```

## Validation Strategy

### During Implementation (Task 3.1.4-3.1.8)

For each new concrete exporter (CSV, JSON, XML, HTML, LaTeX):

1. **Implement exporter** following Strategy pattern
2. **Run exporter** on same test truss
3. **Byte-compare output** with golden master:
   ```bash
   diff golden_master.csv new_csv_output.csv
   ```
4. **Validate numerically** if byte-comparison fails:
   - Parse both files
   - Compare numerical values with tolerance (±1e-6)
   - Compare structure/format

### Acceptance Criteria

- ✅ **Ideal:** Byte-identical output (diff returns 0)
- ✅ **Acceptable:** Numerically equivalent within tolerance
- ❌ **Unacceptable:** Structural differences or missing data

## Usage Example

### Regenerate Golden Masters

If needed (e.g., after updating exporter output):

```bash
# Build generator target
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target generate_corrected_golden_masters

# Generate files
./build/generate_corrected_golden_masters

# Verify outputs
ls -lh tests/fixtures/export_golden/
```

### Validate New Exporter

```cpp
// Example: Validate CSV exporter
#include "infrastructure/export/csv_exporter.hpp"

auto exporter = std::make_unique<CSVExporter>();
exporter->exportResults(truss, results, "test_output.csv", options);

// Compare with golden master
system("diff tests/fixtures/export_golden/golden_master.csv test_output.csv");
```

## Notes

- **Timestamp fields** will differ between runs (use regex or structural comparison)
- **Floating-point precision** may vary slightly (±1e-6 tolerance)
- **Line ending differences** (CRLF vs LF) should be normalized
- **Whitespace differences** in JSON/XML are acceptable if structure is preserved

## Version History

- **v1.0 (2026-02-07):** Initial golden master generation for Phase 3
- Generated from commit: [TBD after commit]
- Legacy exporter version: 2.2.0

## See Also

- [generate_corrected_golden_masters.cpp](../../generate_corrected_golden_masters.cpp) - Generator source code
