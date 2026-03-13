# Example Truss Models

This directory contains example truss structures demonstrating various structural analysis scenarios and design principles.

## Available Examples

### 1. Simple Triangular Truss

**Location:** `simple-triangular-truss/`  
**Complexity:** Beginner  
**Analysis Time:** < 1 second

The fundamental building block - a single triangular truss demonstrating basic structural mechanics.

- **Nodes:** 3
- **Members:** 3
- **Load:** Single point load
- **Learning focus:** Force resolution, equilibrium, support types

**Key concepts:**

- Statically determinate structures
- Tension vs compression
- Basic FEM validation

[See detailed documentation →](simple-triangular-truss/README.md)

---

### 2. Warren Truss Bridge

**Location:** `bridge-truss/`  
**Complexity:** Intermediate  
**Analysis Time:** < 1 second

A realistic two-span bridge using the efficient Warren truss configuration.

- **Nodes:** 9
- **Members:** 14
- **Span:** 20 meters total
- **Learning focus:** Load distribution, truss efficiency, multi-span structures

**Key concepts:**

- Warren truss design principles
- Symmetrical loading
- Support reaction distribution
- Member force patterns in bridges

[See detailed documentation →](bridge-truss/README.md)

---

### 3. Tapered Tower Structure

**Location:** `tower-structure/`  
**Complexity:** Advanced  
**Analysis Time:** < 1 second

A multi-level tapered tower with lateral wind loading, demonstrating vertical cantilever behavior.

- **Nodes:** 9
- **Members:** 17
- **Height:** 20 meters
- **Learning focus:** Lateral loads, tapering strategy, compression-dominant design

**Key concepts:**

- Wind load analysis
- Structural tapering for efficiency
- Bracing effectiveness
- Overturning moment resistance

[See detailed documentation →](tower-structure/README.md)

---

### 4. Pratt Truss Bridge

**Location:** `pratt-truss-bridge/`  
**Complexity:** Intermediate–Advanced  
**Analysis Time:** < 1 second

A classic 6-panel Pratt bridge truss with vertical and diagonal members, demonstrating the signature force distribution of the Pratt configuration.

- **Nodes:** 14
- **Members:** 25
- **Span:** 24 metres
- **Height:** 4 metres
- **Learning focus:** Pratt layout, compression verticals, tension diagonals

**Key concepts:**

- Pratt truss geometry and member role identification
- Compression vs tension chord behaviour
- Through-truss deck loading on bottom chord
- Steel wide-flange sections for bridge chords

[See detailed documentation →](pratt-truss-bridge/README.md)

---

### 5. Warren Truss Bridge (Single-Span)

**Location:** `warren-truss-bridge/`  
**Complexity:** Intermediate  
**Analysis Time:** < 1 second

A pure Warren truss with no vertical members — alternating diagonals create a distinctive zigzag pattern between top and bottom chords.

- **Nodes:** 13
- **Members:** 23
- **Span:** 24 metres
- **Height:** 3 metres
- **Learning focus:** Pure Warren geometry, alternating diagonal forces

**Key concepts:**

- Warren vs Pratt contrast (no verticals)
- Offset top-chord node placement at panel midpoints
- Top-chord deck loading scenario
- Efficient single-section design with channel profiles

[See detailed documentation →](warren-truss-bridge/README.md)

---

### 6. Fink Roof Truss

**Location:** `fink-roof-truss/`  
**Complexity:** Intermediate  
**Analysis Time:** < 1 second

A traditional Fink roof truss in timber (GL28h), featuring a pitched top chord, king post, and the converging Fink web diagonals.

- **Nodes:** 8
- **Members:** 13
- **Span:** 20 metres
- **Ridge height:** 5 metres
- **Learning focus:** Pitched roof geometry, Fink web pattern, timber material

**Key concepts:**

- Fink diagonal arrangement converging at the ridge
- Non-steel material (GL28h glulam timber, E = 12.85 GPa)
- Snow/dead load applied to sloped top chord nodes
- Multiple section types (rafter, chord, web)

[See detailed documentation →](fink-roof-truss/README.md)

---

### 7. Two-Bay Industrial Roof Frame

**Location:** `two-bay-roof-frame/`  
**Complexity:** Advanced  
**Analysis Time:** < 1 second

A symmetric two-bay steel roof frame with a central support column, showcasing multi-span load redistribution and interior column behaviour.

- **Nodes:** 13
- **Members:** 22
- **Total span:** 24 metres (2 × 12 m bays)
- **Ridge height:** 3 metres per bay
- **Learning focus:** Multi-span behaviour, 4-reaction system, shared central column

**Key concepts:**

- Three-support, four-reaction statically determinate system
- Load sharing between two Fink-pattern bays
- Central column carrying combined reactions from both spans
- RHS/SHS hollow section design

[See detailed documentation →](two-bay-roof-frame/README.md)

---

## How to Use These Examples

### Command Line Interface (CLI)

Run analysis and export results to the `data/` directory:

```bash
# Simple triangular truss
./build/src/cli/truss-analysis-cli analyze --file examples/simple-triangular-truss/model.json \
  -o data/simple-truss-results.csv --format csv

# Warren bridge
./build/src/cli/truss-analysis-cli analyze --file examples/bridge-truss/model.json \
  -o data/bridge-truss-results.csv --format csv

# Tower structure
./build/src/cli/truss-analysis-cli analyze --file examples/tower-structure/model.json \
  -o data/tower-results.csv --format csv

# Pratt truss bridge
./build/src/cli/truss-analysis-cli analyze --file examples/pratt-truss-bridge/model.json \
  -o data/pratt-bridge-results.csv --format csv

# Warren truss bridge (single-span, pure Warren)
./build/src/cli/truss-analysis-cli analyze --file examples/warren-truss-bridge/model.json \
  -o data/warren-bridge-results.csv --format csv

# Fink roof truss (timber GL28h)
./build/src/cli/truss-analysis-cli analyze --file examples/fink-roof-truss/model.json \
  -o data/fink-roof-results.csv --format csv

# Two-bay industrial roof frame
./build/src/cli/truss-analysis-cli analyze --file examples/two-bay-roof-frame/model.json \
  -o data/two-bay-roof-results.csv --format csv
```

### Graphical User Interface (GUI)

```bash
# Launch the GUI
./build/src/gui/truss-analysis

# Then: File > Open > Select example model.json
```

### Viewing Results

Analysis results are exported to `data/` directory in CSV format containing:

- Node coordinates and support conditions
- Member connectivity and properties
- Reaction forces at supports
- Member internal forces and stresses
- Nodal displacements
- Member utilization ratios

Open CSV files with:

- Spreadsheet software (Excel, LibreOffice Calc)
- Text editors
- Data analysis tools (Python, MATLAB)

---

## Model File Format

All examples use JSON format with the following structure:

```json
{
  "metadata": { ... },
  "nodes": [ ... ],
  "members": [ ... ],
  "materials": [ ... ],
  "sections": [ ... ],
  "supports": [ ... ],
  "loads": [ ... ]
}
```

See individual example directories for complete model files.

---

## Learning Path

### Recommended Order

1. **Start with Simple Triangular Truss**
   - Understand basic concepts
   - Verify hand calculations
   - Learn the software interface

2. **Progress to Warren Bridge**
   - See how triangulation scales up
   - Understand load distribution
   - Study real-world applications

3. **Advance to Tower Structure**
   - Complex loading scenarios
   - Multi-level analysis
   - Advanced design considerations

4. **Explore Bridge Configurations**

- Compare Pratt and Warren force distributions
- Understand chord roles (tension vs compression)
- Study member utilization ratios across the span

5. **Graduate to Roof Trusses**

- Pitched geometry and sloped chord forces
- Timber vs steel material comparison
- Distributed roof loading patterns

6. **Master Multi-Bay Frames**

- Multi-support reaction systems
- Interior column load accumulation
- Symmetric vs asymmetric loading scenarios

---

## Creating Your Own Models

Use these examples as templates:

1. Copy an existing `model.json`
2. Modify node coordinates
3. Update member connectivity
4. Adjust material properties
5. Define supports and loads
6. Run analysis
7. Review results

### Tips

- Ensure structural stability (proper supports and bracing)
- Check static determinacy: `m = 2n - r` (or `m > 2n - r` for indeterminate)
- Use consistent units throughout
- Start simple and add complexity gradually
- Validate with hand calculations when possible

---

## Validation and Verification

All examples have been validated against:

- Hand calculations (for determinate structures)
- Classical structural analysis theory
- Published engineering references

Expected results are documented in each example's README.

---

## Additional Resources

- **Main Documentation:** `../docs/`
- **User Guide:** `../docs/user/`
- **Theory Reference:** `../docs/architecture/`
- **API Documentation:** `../docs/api/`

---

## Contributing

To add new examples:

1. Create a new directory under `examples/`
2. Include `model.json` with your structure
3. Write a detailed `README.md` explaining the model
4. Validate the analysis results
5. Document expected outcomes and learning objectives

See `../CONTRIBUTING.md` for guidelines.

---

## Quick Reference Table

| Example         | Nodes | Members | DOF | Main Feature      | Complexity        |
| --------------- | ----- | ------- | --- | ----------------- | ----------------- |
| Simple Triangle | 3     | 3       | 6   | Basic mechanics   | ⭐ Beginner       |
| Warren Bridge   | 9     | 14      | 18  | Load distribution | ⭐⭐ Intermediate |
| Tower Structure | 9     | 17      | 18  | Lateral loads     | ⭐⭐⭐ Advanced   |

---

**Happy Analyzing! 🏗️**
