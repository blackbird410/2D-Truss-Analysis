# Analysis Results Data Directory

This directory contains analysis results exported from the 2D Truss Analysis software.

## Current Results

### Example Analysis Results

The following CSV files contain analysis results from the example models:

#### 1. `bridge-truss-results.csv`

**Source:** `examples/bridge-truss/model.json`  
**Structure:** Warren Truss Bridge (9 nodes, 14 members, 20m span)

Contains:

- Node displacements, member forces, support reactions
- Maximum displacement: ~3.7 mm, Maximum stress: ~60 MPa
- All members <25% utilization (safe design)

#### 2. `simple-truss-results.csv`

**Source:** `examples/simple-triangular-truss/model.json`  
**Structure:** Simple Triangular Truss (3 nodes, 3 members)

Contains:

- Basic force resolution into tension/compression
- Maximum displacement: ~0.08 mm, Maximum stress: ~6 MPa
- Validates hand calculations perfectly

#### 3. `tower-results.csv`

**Source:** `examples/tower-structure/model.json`  
**Structure:** Tapered Tower (9 nodes, 17 members, 20m height)

Contains:

- Lateral wind load effects, tower deflection analysis
- Maximum displacement: ~35 mm, Maximum stress: ~72 MPa
- Demonstrates compression-dominant tower behavior

---

## Generating Analysis Results

To export results for any model:

```bash
# CSV format (recommended for spreadsheets)
./build/src/cli/truss-analysis-cli analyze --file [model.json] \
  -o data/[output].csv --format csv

# JSON format (for programmatic use)
./build/src/cli/truss-analysis-cli analyze --file [model.json] \
  -o data/[output].json --format json

# XML format (for data interchange)
./build/src/cli/truss-analysis-cli analyze --file [model.json] \
  -o data/[output].xml --format xml
```

---

## CSV File Structure

Results include these sections:

1. **GEOMETRY** - Node coordinates and member connectivity
2. **MATERIAL PROPERTIES** - Young's modulus, area, density
3. **RESULTS - REACTIONS** - Support reaction forces
4. **RESULTS - DISPLACEMENTS** - Nodal displacements
5. **RESULTS - MEMBER FORCES** - Axial forces and stresses
6. **ANALYSIS METADATA** - Solver information and convergence

---

## Usage Examples

**Open in Excel/LibreOffice:**

```bash
open bridge-truss-results.csv
```

**Python (pandas):**

```python
import pandas as pd
df = pd.read_csv('data/bridge-truss-results.csv', comment='#')
```

**MATLAB:**

```matlab
data = readtable('data/bridge-truss-results.csv', 'CommentStyle', '#');
```

---

## Git Behavior

This directory is git-ignored except for:

- Example result files (for validation and documentation)
- This README
- `.gitkeep` (to preserve directory structure)

User-generated results are not tracked by default.

---

## Best Practices

- Use descriptive filenames: `[model-name]-results.[ext]`
- Include timestamps for multiple runs: `tower-results-2026-02-23.csv`
- Keep important results backed up externally
- Results can be safely deleted; regenerate from source models

---

**Last Updated:** 2026-02-23  
**Software Version:** 3.0.0
