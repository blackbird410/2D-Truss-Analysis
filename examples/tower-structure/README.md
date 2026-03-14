# Tapered Tower Structure Example

A multi-level tapered tower demonstrating:

- Progressive tapering for structural efficiency
- Combined lateral (wind) and vertical loads
- Multi-level diagonal bracing patterns
- Realistic tower design principles

## Structure Details

- **Nodes:** 9
- **Members:** 17
- **Height:** 20 meters (4 levels @ 5m each)
- **Base width:** 4 meters
- **Top width:** 1 meter
- **Taper ratio:** 4:1
- **Type:** Tapered lattice tower with X-bracing

## Member Sections

- **Base columns (heavy):** 100×100mm tube (A = 0.00380 m²)
- **Mid-level bracing:** 75×75mm tube (A = 0.00218 m²)
- **Top level (light):** 50×50mm tube (A = 0.00145 m²)
- **Material:** Structural steel (E = 200 GPa, fy = 250 MPa)

## Support Conditions

- **Nodes 1 & 2 (base):** Fixed supports (restrain x, y)
- **All other nodes:** Free to displace

## Loading Conditions

### Wind Loads (horizontal)

- **Level 3 (nodes 7, 8):** 20 kN each (40 kN total)
- **Level 2 (nodes 5, 6):** 30 kN each (60 kN total)
- **Total horizontal load:** 100 kN

### Vertical Load

- **Top (node 9):** 10 kN downward (equipment/antenna)

## Model Files

This example is provided in two formats:

- **JSON:** `model.json` - Human-readable JSON format with material/section library references
- **XML:** `model.xml` - Structured XML format with the same data model

Both formats are fully compatible and produce identical analysis results.

## How to Run

```bash
# Command line analysis with JSON (CSV export)
./build/src/cli/truss-analysis-cli analyze --file examples/tower-structure/model.json -o data/tower-results.csv --format csv

# Command line analysis with XML (CSV export)
./build/src/cli/truss-analysis-cli analyze --file examples/tower-structure/model.xml -o data/tower-results.csv --format csv

# Validate the model
./build/src/cli/truss-analysis-cli validate --file examples/tower-structure/model.json
./build/src/cli/truss-analysis-cli validate --file examples/tower-structure/model.xml

# GUI mode
./build/src/gui/truss-analysis
# Then: File > Open > examples/tower-structure/model.json (or model.xml)
```

## Expected Results

- **Max displacement:** ~35 mm (lateral at top)
- **Max stress:** ~72 MPa (compression in windward column)
- **Max utilization:** ~29% (well within safe limits)
- **Critical member:** Member 2 (base column, leeward side)
- **Lateral drift ratio:** ~1/575 (very stiff)

## Analysis Output

Results are exported to `data/tower-results.csv` containing:

- Complete geometry with 4 tapered levels
- Member forces showing compression/tension patterns
- Large lateral displacements from wind loading
- Base reaction forces and moments
- Progressive stress distribution through height
- Utilization ratios for structural adequacy check

## Learning Objectives

This example demonstrates:

1. **Lateral Load Resistance:** How towers resist horizontal wind forces
2. **Tapering Strategy:** Heavier sections at base, lighter at top
3. **Bracing Effectiveness:** X-bracing controls lateral deflection
4. **Load Distribution:** Wind loads increase structural demands
5. **Column Behavior:** Compression dominates in vertical members
6. **Bending Effects:** Tower acts like a vertical cantilever

## Key Observations

### Member Force Patterns

- **Windward column (Member 1):** High tension from overturning moment
- **Leeward column (Member 2):** High compression (maximum stress)
- **Diagonal bracing:** Alternating tension/compression
- **Horizontal ties:** Minimal force (connect legs)

### Displacement Pattern

- Progressive lateral displacement increasing with height
- Top displacement dominated by lateral drift
- Base essentially rigid (fixed supports)

### Design Efficiency

- Tapering reduces material use while maintaining strength
- Heavier base sections resist maximum moments
- Lighter top sections adequate for reduced loads
- All members show <30% utilization (safe design)

## Engineering Notes

- This represents a communication tower or transmission tower
- Wind loading is the dominant design consideration
- Tapering is structurally efficient and economical
- X-bracing provides excellent lateral stiffness
- Base fixity is critical for stability
- In practice, foundation design would be critical for these base reactions

## Real-World Applications

- Cell phone towers
- Radio/TV transmission towers
- Wind measurement towers
- Observation towers
- Utility transmission structures

```

```
