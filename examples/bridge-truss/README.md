# Warren Truss Bridge Example

A realistic two-span Warren truss bridge structure demonstrating:

- Classic Warren truss configuration
- Symmetrical loading conditions
- Multiple support types
- Real-world dimensions and materials

## Structure Details

- **Nodes:** 9
- **Members:** 14
- **Total Length:** 20 meters (2 × 10m spans)
- **Height:** 2.165 meters (equilateral triangle pattern)
- **Type:** Warren truss with vertical posts
- **Material:** Structural steel (E = 200 GPa)
- **Section:** 100mm Channel (A = 0.00135 m²)

## Support Conditions

- **Node 1 (Left):** Pinned support (restrains x, y)
- **Node 5 (Center):** Roller support (restrains y only)
- **Node 9 (Right):** Pinned support (restrains x, y)

## Loading

- **Top chord nodes (2, 4, 6, 8):** 30 kN downward
- **Bottom chord nodes (3, 7):** 40 kN downward
- **Total load:** 200 kN

## Model Files

This example is provided in two formats:

- **JSON:** `model.json` - Human-readable JSON format with material/section library references
- **XML:** `model.xml` - Structured XML format with the same data model

Both formats are fully compatible and produce identical analysis results.

## How to Run

```bash
# Command line analysis with JSON (CSV export)
./build/TrussAnalysisCLI analyze --file examples/bridge-truss/model.json -o data/bridge-truss-results.csv --format csv

# Command line analysis with XML (CSV export)
./build/TrussAnalysisCLI analyze --file examples/bridge-truss/model.xml -o data/bridge-truss-results.csv --format csv

# Validate the model
./build/TrussAnalysisCLI validate --file examples/bridge-truss/model.json
./build/TrussAnalysisCLI validate --file examples/bridge-truss/model.xml

# GUI mode
./build/TrussAnalysisGUI
# Then: File > Open > examples/bridge-truss/model.json (or model.xml)
```

## Expected Results

- **Max displacement:** ~3.7 mm (at mid-span bottom chord)
- **Max stress:** ~60 MPa (compression in diagonal members)
- **Max utilization:** ~24% (well within safe limits)
- **Critical members:** Top chord diagonals (Members 7, 14)

## Analysis Output

Results are exported to `data/bridge-truss-results.csv` containing:

- Node coordinates and support conditions
- Member geometry and properties
- Reaction forces at supports
- Member internal forces and stresses
- Node displacements
- Member utilization ratios

## Learning Objectives

This example demonstrates:

1. **Warren Truss Behavior:** Efficient load distribution through triangulation
2. **Load Paths:** How vertical loads create tension/compression in diagonals
3. **Symmetry:** Force distribution in symmetrical structures
4. **Support Reactions:** Different support types and their effects
5. **Structural Efficiency:** High strength-to-weight ratio of truss design

## Engineering Notes

- The Warren truss is one of the most efficient bridge designs
- Diagonal members alternate between tension and compression
- The center roller support allows for thermal expansion
- All members show reasonable utilization (<25%), indicating a well-designed structure
