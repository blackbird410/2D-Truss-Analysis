# Simple Triangular Truss Example

The fundamental building block of truss analysis: a single equilateral triangle.

## Structure Details

- **Nodes:** 3 (vertices of triangle)
- **Members:** 3 (sides of triangle)
- **Base width:** 4 meters
- **Height:** ~3.464 meters (equilateral)
- **Material:** Structural steel (E = 200 GPa, fy = 250 MPa)
- **Section:** 100mm L-angle (A = 0.00095 m²)

## Support Conditions

- **Node 1 (Left):** Pinned support (restrains x, y)
- **Node 2 (Right):** Roller support (restrains y only)
- **Node 3 (Apex):** Free (loaded node)

## Loading

- **Node 3 (apex):** 5 kN downward vertical load

## Model File

See `model.json` for the complete truss definition.

## How to Run

```bash
# Command line analysis with CSV export
./build/TrussAnalysisCLI analyze --file examples/simple-triangular-truss/model.json -o data/simple-truss-results.csv --format csv

# GUI mode
./build/TrussAnalysisGUI
# Then: File > Open > examples/simple-triangular-truss/model.json
```

## Expected Results

- **Max displacement:** ~0.08 mm (vertical at apex)
- **Max stress:** ~6 MPa (compression in inclined members)
- **Max utilization:** ~2.4% (very lightly stressed)
- **Member forces:**
  - Member 1 (base): 2.89 kN tension
  - Members 2 & 3 (inclined): 5.77 kN compression each

## Analysis Output

Results are exported to `data/simple-truss-results.csv` containing:

- Node coordinates and boundary conditions
- Member connectivity and properties
- Support reaction forces
- Member internal forces and stresses
- Nodal displacements
- Safety factors and utilization

## Learning Objectives

This example is ideal for understanding:

1. **Force Resolution:** How a vertical load splits into diagonal forces
2. **Equilibrium:** Basic statics and force balance
3. **Support Types:** Difference between pinned and roller supports
4. **Tension vs Compression:** Base member in tension, inclined members in compression
5. **FEM Basics:** Simplest statically determinate truss for validation

## Theory

### Force Analysis (Hand Calculation)

For verification, the forces can be calculated manually:

- **Vertical equilibrium:** R₁y + R₂y = 5 kN
- **Horizontal equilibrium:** R₁x = 0 (by symmetry)
- **Moment equilibrium:** R₁y = R₂y = 2.5 kN (by symmetry)

Member forces (using method of joints):

- **F₁ (base):** 2.89 kN tension
- **F₂, F₃ (inclined):** 5.77 kN compression

### Static Determinacy

This structure is statically determinate:

- **Check:** m = 2n - r → 3 = 2(3) - 3 ✓
- Can be solved using equilibrium equations alone

## Engineering Notes

- This is the fundamental stable truss unit
- All trusses are built from triangular cells for rigidity
- The base member resists the outward thrust from the inclined members
- Very low stress levels indicate this structure is overdesigned for the applied load
