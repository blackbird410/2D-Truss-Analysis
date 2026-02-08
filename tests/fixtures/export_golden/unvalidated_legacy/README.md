# Unvalidated Legacy Golden Masters

**Status**: ⚠️ **INCOMPLETE - DO NOT USE FOR VALIDATION**  
**Date Isolated**: 2026-02-08  
**Reason**: Data completeness verification failure

---

## ⚠️ CRITICAL WARNING

These golden master files are **INCOMPLETE** and **MUST NOT** be used for validation testing until the corresponding Strategy pattern exporters are implemented and validated.

**Missing from ALL files**: Reactions data (support reactions - MANDATORY for structural analysis)

---

## Contents

This directory contains legacy golden masters that **failed data completeness validation**:

| File                 | Size        | Format | Status                                        |
| -------------------- | ----------- | ------ | --------------------------------------------- |
| `golden_master.html` | 1,087 bytes | HTML   | ❌ Incomplete (only Project + Geometry nodes) |
| `golden_master.tex`  | 764 bytes   | LaTeX  | ❌ Incomplete (only Project + Geometry)       |
| `golden_master.txt`  | 1,549 bytes | TXT    | ❌ Incomplete (missing reactions)             |

---

## Data Completeness Analysis

### HTML (1,087 bytes) - SEVERELY INCOMPLETE

**Sections Present**:

- ✅ Project metadata (partial)
- ⚠️ Geometry (nodes only)

**Missing Critical Sections**:

- ❌ Members geometry (table not generated)
- ❌ Displacements
- ❌ Member Forces
- ❌ **Reactions** (MANDATORY - completely absent)
- ❌ Analysis metadata

**Legacy Code Issue**: `ResultsExporter::exportToHTML()` at line 374-428 only generates:

```cpp
bool ResultsExporter::exportToHTML(...) {
    // Only outputs:
    // - HTML header/style
    // - Project summary div
    // - Nodes table
    // - Closes </body></html>
    // NO OTHER SECTIONS
}
```

**Completeness**: ~25% (2 of 8 required sections, one incomplete)

---

### LaTeX (764 bytes) - SEVERELY INCOMPLETE

**Sections Present**:

- ✅ Project metadata
- ⚠️ Geometry (nodes table only)

**Missing Critical Sections**:

- ❌ Members geometry
- ❌ Displacements
- ❌ Member Forces
- ❌ **Reactions** (MANDATORY - completely absent)
- ❌ Analysis metadata

**Legacy Code Issue**: `ResultsExporter::exportToLaTeX()` at line 318-372 appears to be minimal stub implementation

**Completeness**: ~25% (2 of 8 required sections)

---

### TXT (1,549 bytes) - MISSING REACTIONS

**Sections Present**:

- ✅ Project metadata
- ✅ Geometry (nodes + members - COMPLETE)
- ✅ Analysis Summary
- ✅ Nodal Displacements (max displacement value)
- ✅ Member Forces (values + tension/compression type)

**Missing Critical Sections**:

- ❌ **Reactions** (MANDATORY - explicitly omitted by design)
- ⚠️ Displacements (only max value, not per-DOF values)

**Legacy Code Issue**: `ResultsExporter::exportToText()` at line 198-270 has:

```cpp
// Displacements section - only max value
if (options.includeDisplacements && results.displacements.size() > 0) {
    file << "Maximum displacement: " << results.maxDisplacement << "\n\n";
    // NO per-DOF displacement table
}

// Member forces section - present

// NO reactions section implementation
// (check options.includeReactions is never used)
```

**Completeness**: ~70% (5.5 of 8 required sections, reactions completely missing)

---

## Why These Files Cannot Be Used

### 1. Structural Analysis Fundamentals Violated

**Reactions are MANDATORY** for structural equilibrium verification:

- Sum of applied loads = Sum of reactions (Newton's 3rd law)
- Without reactions, cannot verify analysis correctness
- Exporting incomplete results is a **data integrity violation**

### 2. Inconsistent with Corrected Exporters

CSV, JSON, and XML exporters have been corrected to include reactions. Using incomplete golden masters for HTML/LaTeX/TXT would:

- Create **semantic inconsistency** across formats
- Perpetuate legacy design errors
- Violate the principle: "CSV defines correctness"

### 3. Test Validity Compromised

Golden masters define expected behavior. Incomplete golden masters:

- Cannot catch regressions (no reactions to check)
- May pass tests that should fail
- Provide false confidence in implementation

### 4. Legacy Parity ≠ Correctness

These files represent legacy `ResultsExporter` behavior, which was **incorrect by design**:

- HTML/LaTeX: Minimal stub implementations
- TXT: Explicitly omitted reactions section
- All three: Data completeness was never a design goal

**Achieving parity with incorrect legacy behavior is not the objective.**

---

## Required Actions Before Use

For EACH format (HTML, LaTeX, TXT):

### Step 1: Implement Strategy Pattern Exporter

```cpp
// Example: HTMLExporter
class HTMLExporter : public IResultsExporter {
public:
    bool exportResults(const Truss& truss,
                       const AnalysisResults& results,
                       const std::filesystem::path& filePath,
                       const ExportOptions& options) override {
        // MUST include ALL sections:
        // 1. Project metadata
        // 2. Geometry (nodes + members)
        // 3. Displacements (per-DOF + max)
        // 4. Member forces (values + type)
        // 5. REACTIONS (support reactions - MANDATORY)
        // 6. Analysis metadata (convergence, iterations, etc.)
    }
};
```

### Step 2: Generate New Golden Master

```bash
# Update generate_corrected_golden_masters.cpp
# Add HTMLExporter, LaTeXExporter, TextExporter
./generate_corrected_golden_masters

# Verify output includes reactions:
grep -i "reaction" tests/fixtures/export_golden/golden_master.html
grep -i "reaction" tests/fixtures/export_golden/golden_master.tex
grep -i "reaction" tests/fixtures/export_golden/golden_master.txt
```

### Step 3: Validate Completeness

- Cross-check against CSVExporter output (authoritative reference)
- Verify all 6-8 required sections present
- Confirm reactions data matches CSV exactly
- Byte-compare or numerically validate

### Step 4: Create Comprehensive Tests

- Minimum 15 tests per format (match JSON/XML test coverage)
- **MUST include ReactionsSection test** (enforces presence)
- GoldenMasterEquivalence test (validates completeness)
- Section-specific tests (Displacements, Forces, Reactions, Metadata)

### Step 5: Mark as VALIDATED

Update `VALIDATION_STATUS.md`:

```markdown
| HTML | ✅ **VALIDATED** | ✅ HTMLExporter | ✅ Complete | ✅ Yes | Corrected 2026-02-XX |
```

---

## File Size Projections (After Correction)

Based on corrected CSV/JSON/XML golden masters:

| Format | Current (bytes) | Projected (bytes) | Increase  | Reason                      |
| ------ | --------------- | ----------------- | --------- | --------------------------- |
| HTML   | 1,087           | ~2,500-3,000      | +130-176% | Adding 4-5 missing sections |
| LaTeX  | 764             | ~1,800-2,200      | +136-188% | Adding 4-5 missing sections |
| TXT    | 1,549           | ~1,800-2,000      | +16-29%   | Adding reactions table      |

**Note**: These are estimates. Actual sizes depend on formatting choices (tables vs. lists, indentation, etc.)

---

## Comparison with Validated Golden Masters

### Validated (Safe to Use)

**CSV (945 bytes)**:

- ✅ 8 complete sections
- ✅ Reactions: CSV table with DOF and value columns
- ✅ Authoritative reference

**JSON (1,315 bytes)**:

- ✅ 6 complete sections
- ✅ Reactions: `"reactions": { "values": [3730.56, 14310.52, -3730.56] }`
- ✅ Corrected 2026-02-08

**XML (2,194 bytes)**:

- ✅ 6 complete sections
- ✅ Reactions: `<Reactions><Values><Reaction dof="0">3730.57</Reaction>...</Values></Reactions>`
- ✅ Corrected 2026-02-08

### Unvalidated (DO NOT USE)

**HTML (1,087 bytes)** - This file:

- ❌ 2 incomplete sections (25% completeness)
- ❌ No reactions

**LaTeX (764 bytes)** - This file:

- ❌ 2 sections (25% completeness)
- ❌ No reactions

**TXT (1,549 bytes)** - This file:

- ⚠️ 5.5 sections (70% completeness)
- ❌ No reactions

---

## Historical Context

### Why Were These Files Incomplete?

**Legacy ResultsExporter Design Philosophy** (flawed):

- HTML/LaTeX: "Quick preview formats" - only basic structure
- TXT: "Human-readable summary" - not comprehensive export
- CSV: "Data export format" - complete implementation

**Problem**: This creates **semantic inconsistency**:

- Different formats export different data sets
- Cannot reliably round-trip data
- Violates user expectation of complete results

### Refactoring Corrects This

**New Philosophy**: All export formats are **semantically equivalent**

- Same logical data set in different formats
- CSVExporter defines completeness (authoritative)
- All formats include reactions (mandatory)
- Format differences are syntactic, not semantic

---

## Usage Restrictions

### ✅ ALLOWED

- Reference for historical legacy behavior
- Understanding what legacy system produced
- Documentation of data completeness issues

### ❌ FORBIDDEN

- Using as golden masters for validation tests
- Comparing new exporter output against these files
- Treating as "correct" or "expected" behavior
- Copying structure/content for new implementations

---

## See Also

- [../VALIDATION_STATUS.md](../VALIDATION_STATUS.md) - Complete validation tracking
- [../legacy_incomplete/README.md](../legacy_incomplete/README.md) - Legacy JSON/XML (also incomplete)
- [../../docs/work-logs/2026-02-08-correctness-fix-exporter-completeness.md](../../docs/work-logs/2026-02-08-correctness-fix-exporter-completeness.md) - Breaking change documentation
- [../../src/core/ResultsExporter.cpp](../../src/core/ResultsExporter.cpp) - Legacy implementation (lines 198-428)

---

**Status**: ⚠️ **INCOMPLETE - DO NOT USE**  
**Action Required**: Implement Strategy pattern exporters before use  
**Principle**: Golden masters define truth. Incomplete truth is worse than no truth.
