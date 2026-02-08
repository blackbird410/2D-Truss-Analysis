# Legacy Incomplete Golden Masters

**Status**: ⚠️ ARCHIVED - INCOMPLETE IMPLEMENTATIONS  
**Date Archived**: 2026-02-08  
**Reason**: Correctness fix - these golden masters represent incomplete legacy behavior

---

## Contents

This directory contains the **original incomplete** golden master files that were generated from the legacy `ResultsExporter` class:

- `golden_master.json` (1,237 bytes) - **Missing reactions section**
- `golden_master.xml` (1,071 bytes) - **Missing 4 sections** (displacements, forces, reactions, metadata)

---

## Why These Were Incomplete

### JSON (1,237 bytes)

**Missing**: `reactions` section (support reactions data)

**Original Structure**:

```json
{
  "project": {...},
  "geometry": {...},
  "displacements": {...},
  "memberForces": {...},
  // ❌ MISSING: "reactions": {...}
  "analysis": {...}
}
```

**Reason**: Legacy code had explicit comment: "JSON format should NOT include reactions (legacy behavior)" - this was a design error, not a feature.

---

### XML (1,071 bytes)

**Missing**: 4 critical sections (displacements, forces, reactions, metadata)

**Original Structure**:

```xml
<TrussAnalysisResults>
  <Project>...</Project>
  <Geometry>...</Geometry>
  <!-- ❌ MISSING: <Displacements> -->
  <!-- ❌ MISSING: <MemberForces> -->
  <!-- ❌ MISSING: <Reactions> -->
  <!-- ❌ MISSING: <Analysis> -->
</TrussAnalysisResults>
```

**Reason**: Legacy code had explicit comment: "Legacy XML export only includes Project and Geometry sections" - this was a minimal viable implementation, not complete functionality.

---

## Corrected Versions

The **corrected** golden masters are in the parent directory:

- `../golden_master.json` (1,315 bytes) - **NOW INCLUDES reactions** (+78 bytes, +6.3%)
- `../golden_master.xml` (2,194 bytes) - **NOW COMPLETE with all 6 sections** (+1,123 bytes, +104.9%)

---

## File Size Comparison

| Format | Legacy (bytes) | Corrected (bytes) | Change | Percentage |
| ------ | -------------- | ----------------- | ------ | ---------- |
| JSON   | 1,237          | 1,315             | +78    | +6.3%      |
| XML    | 1,071          | 2,194             | +1,123 | +104.9%    |

---

## Why This Change Was Made

### Fundamental Issue

Legacy `ResultsExporter` had incomplete implementations for JSON and XML formats. These golden masters validated **format compliance** but not **data completeness**.

### CSVExporter as Reference

`CSVExporter` was complete from the start, containing all 8 sections:

1. Project metadata
2. Geometry
3. Properties (placeholder)
4. Loads (placeholder)
5. Displacements
6. Member Forces
7. **Reactions** (mandatory for equilibrium verification)
8. Metadata

### Correctness Principle

**CSV defines correctness. Legacy does NOT.**

All export formats should provide semantic equivalence - the same logical data set in different formats. Partial exports create data loss and prevent downstream verification.

---

## Breaking Change Justification

This was an **intentional, correctness-driven breaking change**:

1. **Structural Analysis Fundamentals**: Reactions are critical for equilibrium verification
2. **Data Integrity**: Export should preserve ALL computed results, not a subset
3. **User Expectation**: "Export analysis results" should mean ALL results
4. **Test-Driven Validation**: Tests now enforce completeness to prevent regression

---

## Usage

**DO NOT** use these legacy files as reference for new implementations. They represent **incomplete** legacy behavior that has been **intentionally corrected**.

**For Historical Reference Only**: These files document what the legacy system produced, helping understand the scope of the correctness fix.

---

## Documentation

For complete details on the correctness fix, see:

- `../../docs/work-logs/2026-02-08-correctness-fix-exporter-completeness.md`

---

**Archive Date**: 2026-02-08  
**Reason**: Correctness fix - incomplete legacy behavior  
**Status**: ⚠️ DO NOT USE FOR VALIDATION
