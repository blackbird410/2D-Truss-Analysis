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
