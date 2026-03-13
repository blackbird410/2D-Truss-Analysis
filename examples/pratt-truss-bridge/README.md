# Pratt Truss Bridge Example

A single-span 6-panel Pratt truss bridge demonstrating the defining characteristic of the Pratt
configuration: vertical members carry compressive forces and diagonal members carry tensile forces
under gravity loading.

## Structure Details

- **Nodes:** 14 (7 bottom chord + 7 top chord)
- **Members:** 25 (6 bottom chord + 6 top chord + 7 verticals + 6 diagonals)
- **Span:** 24 m (6 panels × 4 m)
- **Height:** 4 m (panel aspect ratio 1:1)
- **Chord section:** HEB 200 wide flange (A = 78.1 cm²)
- **Web section:** L150×150×12 equal angle (A = 34.7 cm²)
- **Material:** Structural steel S355 (E = 200 GPa, σ_y = 355 MPa)

## Static Determinacy

```
m + r = 2j  →  25 + 3 = 28 = 2 × 14  ✓
```

## Support Conditions

- **Node 1 (left):** Pinned — restrains x and y
- **Node 7 (right):** Roller — restrains y only

## Loading

Uniform deck loading applied as equivalent point loads at bottom chord panel joints:

| Node | Position          | Load    |
| ---- | ----------------- | ------- |
| 2    | x = 4 m           | 40 kN ↓ |
| 3    | x = 8 m           | 40 kN ↓ |
| 4    | x = 12 m (centre) | 40 kN ↓ |
| 5    | x = 16 m          | 40 kN ↓ |
| 6    | x = 20 m          | 40 kN ↓ |

**Total load:** 200 kN

## Key Structural Behaviour

- **Bottom chord:** tension (deck tie)
- **Top chord:** compression (compression boom)
- **Vertical members:** compression (Pratt characteristic)
- **Diagonal members:** tension (Pratt characteristic — diagonals slope toward centre from top)
- **Max mid-span deflection:** ~5–8 mm (depending on joint conditions)

## How to Run

```bash
# Analyse (JSON input, CSV output)
./build/src/cli/truss-analysis-cli analyze --file examples/pratt-truss-bridge/model.json \
  --output data/pratt-bridge-results.csv --format CSV

# Analyse (XML input)
./build/src/cli/truss-analysis-cli analyze --file examples/pratt-truss-bridge/model.xml \
  --output data/pratt-bridge-results.csv --format CSV

# Validate only
./build/src/cli/truss-analysis-cli validate --file examples/pratt-truss-bridge/model.json

# GUI
./build/src/gui/truss-analysis
# File > Open > examples/pratt-truss-bridge/model.json
```

## Learning Objectives

- Pratt truss diagonal orientation and force sign convention
- How panel geometry (height:span ratio) affects chord forces
- Comparison of chord vs web member utilisation
- Symmetric loading and mid-span deflection behaviour
