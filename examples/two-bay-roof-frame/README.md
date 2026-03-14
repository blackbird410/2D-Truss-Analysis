# Two-Bay Industrial Roof Frame Example

A two-bay industrial building roof structure spanning 24 m on three column supports — a left
column (pinned), a shared central column (roller), and a right column (roller). Each 12 m bay uses
a Fink-style web with converging diagonals and hollow structural section (RHS/SHS) steel members.

This is the most structurally complex example in the repository: it introduces a third support
reaction, asymmetric load paths to the central column, and non-symmetric deformation when
one-sided loading is applied.

## Structure Details

- **Nodes:** 13 (7 bottom chord + 6 top chord)
- **Members:** 22 (8 top chord + 6 bottom chord + 4 verticals + 4 Fink diagonals)
- **Total width:** 24 m (2 × 12 m bays)
- **Bay span:** 12 m per bay
- **Ridge height:** 3 m (pitch 1:2, 26.6°)
- **Top chord section:** RHS 140×80×5 (A = 21.06 cm²)
- **Bottom chord section:** RHS 100×60×4 (A = 11.84 cm²)
- **Web section:** SHS 80×80×4 (A = 11.84 cm²)
- **Material:** Structural steel S355 (E = 200 GPa, σ_y = 355 MPa)

## Static Determinacy

```
m + r = 2j  →  22 + 4 = 26 = 2 × 13  ✓
```

Three supports (pinned + 2 rollers) provide 4 reactions total, which combined with 22 members
exactly satisfies the determinacy condition for 13 joints.

## Support Conditions

- **Node 1 (left column):** Pinned — restrains x and y
- **Node 4 (centre column):** Roller — restrains y only
- **Node 7 (right column):** Roller — restrains y only

## Loading

Symmetrical roof load (dead + snow) applied at top chord panel joints:

| Node | Location                                 | Load    |
| ---- | ---------------------------------------- | ------- |
| 8    | Left bay, rafter interior L (x=4, y=2)   | 30 kN ↓ |
| 9    | Left ridge (x=6, y=3)                    | 30 kN ↓ |
| 10   | Left bay, rafter interior R (x=8, y=2)   | 30 kN ↓ |
| 11   | Right bay, rafter interior L (x=16, y=2) | 30 kN ↓ |
| 12   | Right ridge (x=18, y=3)                  | 30 kN ↓ |
| 13   | Right bay, rafter interior R (x=20, y=2) | 30 kN ↓ |

**Total load:** 180 kN

Under symmetric loading, the central column carries approximately twice the load of each outer
column. Selectively removing loads from one bay to test asymmetric response is a recommended
experiment.

## Key Structural Behaviour

- **Top chord:** primarily compression (rafters)
- **Bottom chord:** primarily tension (ceiling ties and lateral thrust restraint)
- **Fink diagonals:** tension — transfer rafter shear toward ridge
- **Verticals:** compression — hang the bottom chord panel points from the rafters
- **Central column reaction:** large vertical — redistributes load from both bays
- **Outer column horizontal reaction (pinned, node 1):** absorbs horizontal thrust from both bays

## How to Run

```bash
# Analyse (JSON input, CSV output)
./build/src/cli/truss-analysis-cli analyze --file examples/two-bay-roof-frame/model.json \
  --output data/two-bay-roof-results.csv --format CSV

# Analyse (XML input)
./build/src/cli/truss-analysis-cli analyze --file examples/two-bay-roof-frame/model.xml \
  --output data/two-bay-roof-results.csv --format CSV

# Validate only
./build/src/cli/truss-analysis-cli validate --file examples/two-bay-roof-frame/model.json

# GUI
./build/src/gui/truss-analysis
# File > Open > examples/two-bay-roof-frame/model.json
```

## Learning Objectives

- Multi-span truss with three support reactions
- Effect of central column on load redistribution and reaction magnitudes
- Comparison of force patterns in left vs right bay under symmetric load
- Asymmetric loading: modify loads on one bay only and observe deflection and stress gradient
- Hollow structural section (RHS/SHS) member sizing for industrial applications
