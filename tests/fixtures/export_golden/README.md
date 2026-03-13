# Golden Master Export Files

**Last Updated:** 2026-03-13  
**Purpose:** Canonical reference outputs for exporter verification  
**Validated Formats:** CSV, JSON, XML, HTML, LaTeX, TXT

## Overview

This directory contains the canonical export outputs used by automated tests to validate serialization structure, field completeness, and deterministic formatting behavior.

These files are consumed by exporter unit and integration tests to verify that generated output remains compatible with the current export contract.

## File Inventory

| File                 | Format | Role                                        |
| -------------------- | ------ | ------------------------------------------- |
| golden_master.csv    | CSV    | Canonical delimited tabular export          |
| golden_master.json   | JSON   | Canonical structured export                 |
| golden_master.xml    | XML    | Canonical hierarchical export               |
| golden_master.html   | HTML   | Canonical report rendering export           |
| golden_master.tex    | LaTeX  | Canonical publication-oriented export       |
| golden_master.txt    | TXT    | Canonical plain-text summary export         |
| CHECKSUMS.md         | Text   | Reference checksums for generated artifacts |
| VALIDATION_STATUS.md | Text   | Current fixture validation status           |

## Export Contract Coverage

The golden masters include the full export sections used by the current implementation:

- Geometry
- Properties
- Loads
- Displacements
- Member forces
- Reactions
- Stresses
- Utilization
- Metadata

## Regeneration

Regenerate the files when export schema or formatting rules change intentionally.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target generate_corrected_golden_masters
./build/generate_corrected_golden_masters
```

## Verification

Quick local verification example:

```bash
diff tests/fixtures/export_golden/golden_master.csv /tmp/new_output.csv
```

For floating-point formatting changes, use a tolerance-aware comparator in tests instead of raw byte comparison.

## Notes

- Timestamp fields in metadata may vary when explicitly enabled by export options.
- Normalize line endings before cross-platform comparisons.
- Keep fixture updates and exporter changes in the same pull request.
