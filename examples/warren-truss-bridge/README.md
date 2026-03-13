# Warren Truss Bridge Example

A single-span pure Warren truss bridge with no vertical members. The alternating diagonal pattern
produces the characteristic zigzag geometry. Unlike the Pratt truss, both load directions occur in
the diagonal members depending on panel position relative to mid-span.

This example uses a single uniform section throughout, making it straightforward to compare member
forces across the entire structure.

## Structure Details

- **Nodes:** 13 (7 bottom chord + 6 top chord)
- **Members:** 23 (6 bottom chord + 5 top chord + 12 diagonals)
- **Span:** 24 m (6 panels × 4 m)
- **Height:** 3 m (top nodes at panel midpoints)
- **Section:** UPN 120 channel (A = 16.9 cm²)
- **Material:** Structural steel S275 (E = 200 GPa, σ_y = 275 MPa)

## Static Determinacy

```
m + r = 2j  →  23 + 3 = 26 = 2 × 13  ✓
```

## Support Conditions

- **Node 1 (left):** Pinned — restrains x and y
- **Node 7 (right):** Roller — restrains y only

## Loading

Uniform deck loading via top chord panel joints:

| Node | x [m] | Load    |
| ---- | ----- | ------- |
| 8    | 2     | 30 kN ↓ |
| 9    | 6     | 30 kN ↓ |
| 10   | 10    | 30 kN ↓ |
| 11   | 14    | 30 kN ↓ |
| 12   | 18    | 30 kN ↓ |
| 13   | 22    | 30 kN ↓ |

**Total load:** 180 kN

## Key Structural Behaviour

- **Bottom chord:** tension throughout (increases toward mid-span)
- **Top chord:** compression throughout (increases toward mid-span)
- **Diagonals near supports:** higher forces (shear-dominant zone)
- **Diagonals near mid-span:** lower forces
- **No vertical members:** Warren characteristic — efficient but less stiff than Pratt for concentrated loads

## Contrast with Pratt Truss

| Feature     | Warren (this)                       | Pratt                       |
| ----------- | ----------------------------------- | --------------------------- |
| Verticals   | None                                | Present (compression)       |
| Diagonals   | Both tension + compression          | Tension only                |
| Web members | Fewer (23 vs 25)                    | More                        |
| Typical use | Road bridges, light to medium spans | Rail bridges, heavier loads |

## How to Run

```bash
# Analyse (JSON input, CSV output)
./build/src/cli/truss-analysis-cli analyze --file examples/warren-truss-bridge/model.json \
  --output data/warren-bridge-results.csv --format CSV

# Analyse (XML input)
./build/src/cli/truss-analysis-cli analyze --file examples/warren-truss-bridge/model.xml \
  --output data/warren-bridge-results.csv --format CSV

# Validate only
./build/src/cli/truss-analysis-cli validate --file examples/warren-truss-bridge/model.json

# GUI
./build/src/gui/truss-analysis
# File > Open > examples/warren-truss-bridge/model.json
```

## Learning Objectives

- Pure Warren truss diagonal force distribution
- Comparison of tension vs compression member patterns at each panel
- Effect of removing vertical members on deflection and force distribution
- How top-chord loading (deck bridge) affects bottom chord tension gradient
