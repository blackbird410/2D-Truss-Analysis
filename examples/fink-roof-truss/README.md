# Fink Roof Truss Example

A 20 m span residential/industrial-style Fink roof truss using structural timber. The Fink (or fan)
truss is the most widely manufactured roof truss type, distinguished by its Fink diagonals that
converge at the ridge, subdividing each rafter panel and reducing its effective span.

This example uses GL28h glulam timber sections, making it suitable for comparison with steel
examples and for exploring how material stiffness affects deflection.

## Structure Details

- **Nodes:** 8
- **Members:** 13 (4 rafters + 4 bottom chord + 3 verticals/king post + 2 Fink diagonals)
- **Span:** 20 m
- **Ridge height:** 5 m (pitch 1:2 = 26.6°)
- **Rafter section:** 120×200 mm (A = 240 cm²)
- **Bottom chord section:** 100×175 mm (A = 175 cm²)
- **Web section:** 75×150 mm (A = 112.5 cm²)
- **Material:** GL28h structural glulam (E = 12.85 GPa, characteristic bending strength 28 MPa)

## Static Determinacy

```
m + r = 2j  →  13 + 3 = 16 = 2 × 8  ✓
```

## Support Conditions

- **Node 1 (left eave):** Pinned — restrains x and y
- **Node 5 (right eave):** Roller — restrains y only

## Loading

Combined dead + snow load applied as equivalent point loads at top chord panel joints:

| Node | Location                            | Load    |
| ---- | ----------------------------------- | ------- |
| 6    | Left rafter interior (x=5, y=2.5)   | 40 kN ↓ |
| 7    | Ridge (x=10, y=5)                   | 20 kN ↓ |
| 8    | Right rafter interior (x=15, y=2.5) | 40 kN ↓ |

**Total load:** 100 kN (symmetric)

## Key Structural Behaviour

- **Rafters (top chord):** compression (sloped bending-like axial compression)
- **Bottom chord (tie beams):** tension — prevents the eaves from spreading
- **King post (node 3–7):** compression — supports the centre of the bottom chord
- **Fink diagonals:** tension — the signature members of the Fink pattern
- **Symmetric loading:** equal reactions of 50 kN at each eave

## How to Run

```bash
# Analyse (JSON input, CSV output)
./build/src/cli/truss-analysis-cli analyze --file examples/fink-roof-truss/model.json \
  --output data/fink-roof-results.csv --format CSV

# Analyse (XML input)
./build/src/cli/truss-analysis-cli analyze --file examples/fink-roof-truss/model.xml \
  --output data/fink-roof-results.csv --format CSV

# Validate only
./build/src/cli/truss-analysis-cli validate --file examples/fink-roof-truss/model.json

# GUI
./build/src/gui/truss-analysis
# File > Open > examples/fink-roof-truss/model.json
```

## Learning Objectives

- Roof truss member force patterns (compression in rafters, tension in ties)
- Effect of Fink diagonals on rafter effective span
- Timber vs steel material stiffness comparison
- Non-parallel chord geometry and its effect on deformation display
