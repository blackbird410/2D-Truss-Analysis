# 2D Truss Analysis - Professional Structural Analysis Software

[![Release](https://img.shields.io/github/v/release/blackbird410/2D-Truss-Analysis-cpp)](https://github.com/blackbird410/2D-Truss-Analysis-cpp/releases)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS-lightgrey.svg)](https://www.apple.com/macos/)
[![Qt](https://img.shields.io/badge/Qt-6.9-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)

A professional-grade 2D truss structural analysis application built with modern C++20 and Qt6, featuring an intuitive interactive drawing interface and robust computational engine.

## Overview

Before transitioning to a career in Computer Science, I pursued my studies as a third-year Civil Engineering student. During this time, I constantly pondered how I could leverage my coding skills within the realm of my academic pursuits, seeking to create something valuable not only for myself but also for fellow students. One particular structural aspect that captivated my interest was the truss.

This C++ version builds upon the original Python implementation, offering improved computational efficiency and a more robust architecture. The software is designed to comprehensively analyze truss structures, providing insights into support reactions, node displacements, and presenting a visual representation of the deformed structure.

I firmly believe that this tool will prove invaluable to second and third-year Civil Engineering students enrolled in Structural Analysis courses. It will assist them in validating their manual calculations, fostering a deeper understanding of the subject matter.

This repository serves as a centralized hub housing all the project's implementation files.

## What is a truss ?
![A bridge using a truss as base structure to supports loads.](/images/RRTrussBridgeSideView.jpg)

To understand the functionality of this program, it's essential to grasp the concept of a truss. A truss is essentially an assembly of structural members, typically beams, interconnected at nodes, forming a stable framework. In simpler terms, trusses consist of bars arranged within a two-dimensional Cartesian system. They are a prevalent structural element found in applications such as steel buildings and bridges, including examples like railway bridges and transmission towers.

Trusses can exist in two primary forms: as 2D trusses, entirely confined to a single plane, or as 3D trusses, which extend into three-dimensional space. Specifically, when all the truss members and applied loads are situated within a single plane, we refer to it as a plane truss.

## What are the components of the truss?
A truss fundamental components are: 
- The **nodes or joints**, where the loads and support reactions are applied.
- The **members or beams**, which are connected only at their ends by frictionless hinges in plane trusses 
and by frictionless ball-and-socket joints in space trusses. In a truss, they are subjected only to axial 
forces (compression or tension).

## Computational requirements:
* Prior to undertaking any computations, it's imperative to evaluate the internal stability of a truss. This evaluation ensures that the truss, when detached from its supports, maintains its shape and remains a structurally rigid entity. In this context, 'internal' pertains to the count and configuration of the members intrinsic to the truss itself.

It's worth noting that instability resulting from inadequate external support or incorrect placement of external supports is termed 'external instability.

> [!NOTE]
> The internal stability of a determinate plane truss is assessed by this equation :
> **m + r = 2j** 

Where :   
- **m** : number of members or beams
- **r** : number of support reactions
- **j** : number of nodes or joints

> [!NOTE]
> **m + r > 2j** is also stable, but is considered ***statically indeterminate***, case which will not be treated by 
our program.

* We consider a truss to be *statically determinate if the forces in all its members, as well as all the external 
reactions, can be determined by using the equations of equilibrium*.

## Equations of Condition for Plane Truss
The types of connections used to connect rigid portions of internally unstable structures 
provide equations of condition that, along with the three equilibrium equations, can be 
used to determine the reactions needed to constrain such structures fully. Such equations 
are :
- Because an internal hinge cannot transmit moment, it provides an equation of condition :
    + $\sum{M} = 0$, which is the sum of the moments in all nodes.
- Since these parallel (horizontal) bars cannot transmit force in the direction perpendicular 
to them, this type of connection provides an equation of condition:
    + $\sum{F} = 0$, which is the sum of forces in each nodes.

## Method of Computation
In this program, we use the **Stiffness Method**, more precisely the **displacement method** to 
analyze the truss. 

In this program, we employ the Stiffness Method, specifically the displacement method, to conduct truss analysis.

The Stiffness Method necessitates breaking down the structure into discrete finite elements and designating their endpoints as nodes. In the context of truss analysis, these finite elements correspond to individual truss members, while the nodes signify the joints or connections. We determine the force-displacement characteristics of each element and subsequently interrelate them through the force equilibrium equations established at the nodes. These interconnected relationships are then consolidated into what is referred to as the structure's **stiffness matrix**, denoted as ***K**.

Once the stiffness matrix is defined, we can ascertain the unknown displacements of the nodes under any given load applied to the structure. With knowledge of these displacements, we can subsequently compute both the external and internal forces within the structure by leveraging the force-displacement relations for each individual member.

## 🎯 Key Features

🎨 **Interactive Drawing Canvas**
- Mouse-based node and member placement
- Real-time visual feedback during design
- Snap-to-grid functionality for precision
- Zoom and pan capabilities

🔧 **Advanced Analysis Engine**
- Direct stiffness method implementation
- Support for various load types and boundary conditions
- Automated calculation of displacements, forces, and reactions
- Material and section property management

📊 **Professional Results Display**
- Comprehensive results tables
- Visual deformation plots
- Force diagrams and stress visualization
- Export capabilities for reports

🚀 **Modern Technology Stack**
- **Language**: C++20 with modern standards
- **GUI Framework**: Qt6 (Core, Widgets, GUI)
- **Linear Algebra**: Eigen3 library
- **Build System**: CMake 3.20+
- **Platform**: Cross-platform (macOS, Linux)

## Requirements

- CMake 3.16 or higher
- Qt6 (Qt6Core, Qt6Widgets, Qt6Charts)
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Eigen3 library for matrix operations

## 🚀 Quick Start

### One-Step Installation (Linux) - **Recommended**

```bash
# Clone and install in one command
git clone https://github.com/blackbird410/2D-Truss-Analysis-cpp.git
cd 2D-Truss-Analysis-cpp
./install.sh
```

**Installation Options:**
```bash
./install.sh           # System-wide installation (requires sudo)
./install.sh --user    # User installation (~/.local)
./install.sh --prefix /opt/truss  # Custom location
./install.sh --help    # See all options
```

### Installation (macOS)

**Recommended Method:**
```bash
./install_macos.sh
```

**Alternative Methods:**
- See [INSTALL_MACOS.md](INSTALL_MACOS.md) for comprehensive guide
- See [QUICK_INSTALL.md](QUICK_INSTALL.md) for quick reference

**Code Signing Note:** The application is automatically signed with ad-hoc signatures during build. When first launching, you may need to:
1. Right-click on the app → "Open"
2. Click "Open" in the security dialog
3. This only needs to be done once

### Building from Source

```bash
# Install dependencies using Homebrew
brew install cmake qt@6 eigen

# Clone the repository
git clone https://github.com/blackbird410/2D-Truss-Analysis-cpp.git
cd 2D-Truss-Analysis-cpp

# Create build directory
mkdir build && cd build

# Configure and build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.ncpu)
```

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install cmake qt6-base-dev qt6-charts-dev libeigen3-dev build-essential

# Clone and build (same as macOS)
git clone https://github.com/blackbird410/2D-Truss-Analysis-cpp.git
cd 2D-Truss-Analysis-cpp
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Windows
```cmd
# Install Qt6 and CMake from their official websites
# Or use vcpkg for dependencies

git clone https://github.com/blackbird410/2D-Truss-Analysis-cpp.git
cd 2D-Truss-Analysis-cpp
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

## Usage

### GUI Application
Run the GUI version:
```bash
./TrussAnalysisGUI  # On Unix systems
# or
TrussAnalysisGUI.exe  # On Windows
```

### Command Line Interface
Run the CLI version:
```bash
./TrussAnalysisCLI --example    # Run example analysis
./TrussAnalysisCLI --help       # Show usage information
```

## 📚 Documentation

- **[Installation Guide](INSTALL_MACOS.md)**: Complete macOS installation instructions
- **[Quick Install](QUICK_INSTALL.md)**: Fast installation reference
- **[Code Signing Solutions](CODE_SIGNING_SOLUTIONS.md)**: Troubleshooting guide

## 🤝 Contributing

Contributions are welcome! Please follow our [Conventional Commits](https://www.conventionalcommits.org/) standard:

- `feat:` New features
- `fix:` Bug fixes
- `docs:` Documentation changes
- `style:` Code style changes
- `refactor:` Code refactoring
- `test:` Adding tests
- `build:` Build system changes

## 📊 Performance

- **Startup Time**: < 2 seconds
- **Analysis Speed**: 1000+ elements in < 1 second
- **Memory Usage**: ~50MB typical, ~100MB with large models
- **Platform**: Optimized for Apple Silicon and Intel processors

## 🆔 Version History

- **v2.2.0** (2025-06-30): Cross-platform production release with CI/CD
- **v2.1.3** (2025-06-30): Code signing integration and macOS fixes
- **v2.1.1** (2025-06-30): Production release with all critical fixes
- **v2.1.0** (2025-06-30): Interactive drawing widget implementation
- **v2.0.0** (2025-06-29): Major GUI redesign and Qt6 migration

## Related Projects

- [2D Truss Analysis (Python)](https://github.com/blackbird410/2D_Truss_Analysis) - Original Python implementation

---

**Made with ❤️ for the civil engineering community**
