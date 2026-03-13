# 2D Truss Analysis - Professional Structural Analysis Software

[![Release](https://img.shields.io/github/v/release/blackbird410/2D-Truss-Analysis)](https://github.com/blackbird410/2D-Truss-Analysis/releases)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://www.linux.org/)
[![Qt](https://img.shields.io/badge/Qt-6.9-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Tests](https://img.shields.io/badge/tests-1603%20passing-brightgreen.svg)](tests/)

> **✅ STATUS**: Production-ready v3.0.0 — clean layered C++20/Qt6 architecture, 1603 passing tests, 88.3% line / 91.9% function / 51.8% branch coverage, FHS-compliant Linux packaging, AppStream metainfo for software centre discovery.

A professional-grade 2D truss structural analysis application built with modern C++20 and Qt6, featuring an intuitive interactive drawing interface, robust computational engine, and clean layered architecture following SOLID principles and industry best practices.

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

- Prior to undertaking any computations, it's imperative to evaluate the internal stability of a truss. This evaluation ensures that the truss, when detached from its supports, maintains its shape and remains a structurally rigid entity. In this context, 'internal' pertains to the count and configuration of the members intrinsic to the truss itself.

It's worth noting that instability resulting from inadequate external support or incorrect placement of external supports is termed 'external instability.

> [!NOTE]
> The internal stability of a determinate plane truss is assessed by this equation :
> **m + r = 2j**

Where :

- **m** : number of members or beams
- **r** : number of support reactions
- **j** : number of nodes or joints

> [!NOTE]
> **m + r > 2j** is also stable, but is considered **_statically indeterminate_**, case which will not be treated by
> our program.

- We consider a truss to be _statically determinate if the forces in all its members, as well as all the external
  reactions, can be determined by using the equations of equilibrium_.

## Equations of Condition for Plane Truss

The types of connections used to connect rigid portions of internally unstable structures
provide equations of condition that, along with the three equilibrium equations, can be
used to determine the reactions needed to constrain such structures fully. Such equations
are :

- Because an internal hinge cannot transmit moment, it provides an equation of condition :
  - $\sum{M} = 0$, which is the sum of the moments in all nodes.
- Since these parallel (horizontal) bars cannot transmit force in the direction perpendicular
  to them, this type of connection provides an equation of condition: + $\sum{F} = 0$, which is the sum of forces in each nodes.

## Method of Computation

In this program, we use the **Stiffness Method**, more precisely the **displacement method** to
analyze the truss.

In this program, we employ the Stiffness Method, specifically the displacement method, to conduct truss analysis.

The Stiffness Method necessitates breaking down the structure into discrete finite elements and designating their endpoints as nodes. In the context of truss analysis, these finite elements correspond to individual truss members, while the nodes signify the joints or connections. We determine the force-displacement characteristics of each element and subsequently interrelate them through the force equilibrium equations established at the nodes. These interconnected relationships are then consolidated into what is referred to as the structure's **stiffness matrix**, denoted as **\*K**.

Once the stiffness matrix is defined, we can ascertain the unknown displacements of the nodes under any given load applied to the structure. With knowledge of these displacements, we can subsequently compute both the external and internal forces within the structure by leveraging the force-displacement relations for each individual member.

## Architecture Overview

The application follows **Clean Architecture** principles with strict layer separation and dependency inversion:

```
┌─────────────────────────────────────────────────────────┐
│                    Presentation Layer                   │
│  (GUI: Qt6 Widgets, MVC Controllers, Qt Item Models)    │
└────────────────────┬────────────────────────────────────┘
                     │ DTOs, Interfaces
┌────────────────────▼────────────────────────────────────┐
│                   Application Layer                     │
│     (Services, Facades, Use Cases, DTOs, Results)       │
└────────────────────┬────────────────────────────────────┘
                     │ Domain Models
┌────────────────────▼────────────────────────────────────┐
│                     Domain Layer                        │
│  (Entities: Truss, Node, Member; Analysis; Validation)  │
└────────────────────┬────────────────────────────────────┘
                     │ Interfaces
┌────────────────────▼────────────────────────────────────┐
│                 Infrastructure Layer                    │
│        (File I/O, Export, Logging, Persistence)         │
└─────────────────────────────────────────────────────────┘
```

**MVC Architecture in GUI Layer:**

- **Model**: Qt Item Models (`NodeTableModel`, `MemberTableModel`, `ResultsTableModel`, `ValidationListModel`) backed by `ITrussView` / `IAnalysisResultsView` read interfaces
- **View**: Qt6 widgets displaying data (`MainWindow`, `TrussCanvasWidget`, `InspectorPanel`, `ResultsDockPanel`, etc.)
- **Controller**: `MainWindowController` owns five sub-controllers (`CanvasController`, `InspectorController`, `AnalysisController`, `ProjectController`, `ExportController`) and all models; coordinates all user actions through `ITrussAnalysisFacade`

**Dependency Injection:**

- Controllers depend on `ITrussAnalysisFacade` — a single stable facade interface
- Enables unit testing with `MockTrussAnalysisFacade`
- Production code uses `TrussAnalysisFacade` concrete implementation

**Data Transfer Objects (DTOs):**

- GUI receives `NodeView`, `MemberView` DTOs via read interfaces, not domain entities
- Complete decoupling between GUI and domain layers
- Stable API contract independent of domain model changes

## Key Features

**Interactive Drawing Canvas**

- Mouse-based node and member placement with grid snapping (0.25 m)
- Real-time visual feedback: tension (blue), compression (orange), yield (red)
- Select, move, and delete nodes/members via click or `Delete` key
- Zoom with mouse wheel; pan by holding `Space` + drag
- Auto-zoom viewport to fit geometry on file load
- Display modes: Geometry, Stress Ratio (colour-mapped), Deformed Shape (auto-scaled)

**Keyboard Shortcuts**

| Key      | Action                      |
| -------- | --------------------------- |
| `N`      | Add Node mode               |
| `M`      | Add Member mode             |
| `Esc`    | Select mode                 |
| `Delete` | Delete selected node/member |
| `Ctrl+S` | Save project                |
| `Ctrl+O` | Open project                |
| `Ctrl+N` | New project                 |
| `F5`     | Run analysis                |
| `Z`      | Zoom to fit                 |

**Advanced Analysis Engine**

- Clean Architecture with Qt MVC pattern and interface-based dependency injection
- Testable controllers via `ITrussAnalysisFacade` mock
- Application facade for simplified API (`TrussAnalysisFacade`)
- Direct stiffness method implementation
- Support for pinned, roller-X, and roller-Y boundary conditions
- Automated calculation of displacements, forces, and reactions
- Material and section property management via `InspectorPanel`
- Structural stability validation (static determinacy checks)

**Professional Results Display**

- Comprehensive results tables
- Visual deformation plots
- Force diagrams and stress visualization
- Export capabilities for reports

**Modern Technology Stack**

- **Language**: C++20 with modern standards
  - **GUI Framework**: Qt6 (Core, Widgets, GUI) with MVC pattern
- **Linear Algebra**: Eigen3 library
- **Testing**: Google Test (GTest) + GoogleMock for unit/integration tests
- **Build System**: CMake 3.20+ with production Makefile wrapper
- **Code Quality**: clang-format, clang-tidy, Prettier, EditorConfig
- **Coverage**: lcov/genhtml with automated report generation
- **Platform**: Linux (Ubuntu 22.04+, Fedora, Arch)
- **Architecture**: Clean Architecture with SOLID principles

## Requirements

- CMake 3.20 or higher
- Qt6 (Qt6Core, Qt6Widgets, Qt6Charts)
- C++20 compatible compiler (GCC 10+, Clang 10+)
- Eigen3 library for matrix operations
- Linux operating system

## Quick Start

### Building from Source (Production Makefile Wrapper)

```bash
# Quick build (uses optimized defaults)
make build

# Run all tests
make test

# Generate coverage report
make coverage

# Optional install (system-wide)
sudo make install

# See all available targets
make help
```

**Build System Features:**

- **Makefile Wrapper**: Developer-friendly interface to CMake
- **Auto-detection**: Ninja (preferred) or Make backend
- **Parallel Builds**: Automatic CPU core detection
- **Isolated Build Modes**: Separate directories for release/debug/coverage
- **Code Quality**: Integrated clang-format, clang-tidy, Prettier
- **CI/CD Ready**: Non-interactive targets with proper exit codes

### Alternative: Direct CMake Build (Linux only)

> Note: This project currently supports Linux only. Windows and macOS builds and binaries (for example, `.exe` launchers such as `TrussAnalysisGUI.exe`) are not supported.

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install cmake qt6-base-dev libeigen3-dev build-essential

# Clone the repository
git clone https://github.com/blackbird410/2D-Truss-Analysis.git
cd 2D-Truss-Analysis

# Build the project
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run tests
cd build && ctest

# Install (optional)
sudo cmake --install build
```

### Alternative: Fedora/RHEL

```bash
# Install dependencies
sudo dnf install cmake qt6-qtbase-devel qt6-qtcharts-devel eigen3-devel gcc-c++

# Build as above
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Alternative: Arch Linux

```bash
# Install dependencies
sudo pacman -S cmake qt6-base qt6-charts eigen gcc

# Build as above
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Docker Deployment (Recommended for Production)

```bash
# Production: CLI-only optimized image (125MB)
make docker-build
make docker-run

# Development: Full toolchain with Qt6 (1.94GB)
make docker-build-dev
make docker-dev

# Inside development container
setup  # Interactive build configuration
```

**Docker Features:**

- **Production Image**: 125MB CLI-only build (75% better than target)
- **Development Image**: Complete toolchain (gcc 13.3.0, cmake 3.28, Qt6 Charts 6.4.2)
- **Multi-platform**: Supports linux/amd64, linux/arm64
- **Security**: Non-root execution, multi-stage builds, Trivy scanning
- **CI/CD**: Automated builds via GitHub Actions

See [docker/README.md](docker/README.md) for comprehensive Docker usage guide.

## Usage

### GUI Application

Run the GUI version:

```bash
./TrussAnalysisGUI
```

### Command Line Interface

The CLI version supports comprehensive structural analysis workflows with multiple commands:

```bash
# Display help and available commands
./TrussAnalysisCLI help

# Run example analysis (built-in 3-member truss)
./TrussAnalysisCLI example
./TrussAnalysisCLI -v example        # With verbose output

# Analyze truss structure from file
./TrussAnalysisCLI analyze --file truss.json
./TrussAnalysisCLI analyze -f truss.json --output results.json
./TrussAnalysisCLI analyze -f truss.json -o results.xml --format XML

# Validate truss structure without analysis
./TrussAnalysisCLI validate --file truss.json
./TrussAnalysisCLI validate -f truss.json --verbose

# Export analysis results to various formats
./TrussAnalysisCLI export --truss truss.json --results analysis.json --output report.html
./TrussAnalysisCLI export -t truss.json -r results.json -o report.pdf --format LaTeX
```

**Available Commands:**

- `example` - Run hardcoded example truss analysis (3-member truss)
- `analyze` - Analyze truss structure from file (supports JSON, XML, CSV formats)
- `validate` - Validate truss structure from file without performing analysis
- `export` - Export analysis results to various formats (JSON, XML, CSV, HTML, LaTeX, TSV, TXT)
- `help` - Display usage information and available commands

**Supported File Formats:**

- **Input**: JSON, XML, CSV
- **Output**: JSON, XML, CSV, TSV, TXT, HTML, LaTeX
- **Export**: All output formats plus automatic format detection from file extensions

## For Developers

**Extending the GUI:**

The GUI layer follows Qt MVC pattern with `ITrussAnalysisFacade` dependency injection. To add new features:

1. **Create a Controller** (handles user actions, composes inside `MainWindowController`):

   ```cpp
   class NewFeatureController : public QObject {
       truss::interface::ITrussAnalysisFacade& m_facade;
   public:
       NewFeatureController(truss::interface::ITrussAnalysisFacade& facade, QObject* parent);
       void handleUserAction();
   signals:
       void somethingChanged();
   };
   ```

2. **Create or Update Widget** (displays UI, connects to controller signals):

   ```cpp
   class NewWidget : public QWidget {
       NewFeatureController* m_controller;
       NewFeaturePresenter* m_presenter;
   public:
       NewWidget(NewFeatureController* ctrl, NewFeaturePresenter* pres);
   };
   ```

3. **Write Unit Tests** using the facade mock:
   ```cpp
   TEST(NewFeatureControllerTest, HandlesAction) {
       MockTrussAnalysisFacade mockFacade;
       NewFeatureController controller(mockFacade);
       controller.handleUserAction();
       EXPECT_TRUE(mockFacade.someMethodCalled());
   }
   ```

**Testing:**

```bash
# Run all tests
cd build && ctest

# Run specific test suite
./unit_tests --gtest_filter="ControllerTests.*"

# Run with verbose output
ctest --verbose
```

**Architecture Principles:**

- **Dependency Inversion**: Controllers depend on interfaces, not concrete classes
- **Single Responsibility**: Each component has one clear purpose
- **DTO Pattern**: Use Data Transfer Objects to decouple GUI from domain
- **Testability**: All controllers unit-testable via mocking

## Documentation

- **[System Overview](docs/architecture/system-overview.md)**: High-level architecture, CMake targets, technology stack
- **[Module Structure](docs/architecture/module-structure.md)**: Per-layer component descriptions and test architecture
- **[GUI Architecture](docs/architecture/gui-architecture.md)**: Qt MVC structure, controllers, signal/slot conventions
- **[Development Guide](docs/development/development-guide.md)**: Build, test, coverage, and CI/CD workflows
- **[Work Logs](docs/work-logs/)**: Historical session records

## Contributing

Contributions are welcome! Please follow our [Conventional Commits](https://www.conventionalcommits.org/) standard:

- `feat:` New features
- `fix:` Bug fixes
- `docs:` Documentation changes
- `style:` Code style changes
- `refactor:` Code refactoring
- `test:` Adding tests
- `build:` Build system changes

## Performance

- **Startup Time**: < 2 seconds
- **Analysis Speed**: 1000+ elements in < 1 second
- **Memory Usage**: ~50MB typical, ~100MB with large models
- **Platform**: Optimized for Linux (x86_64 and ARM64)

## Version History

- **v3.0.0** (2026): Production-ready release — Clean Architecture, Qt6 MVC, SOLID principles, Google Test, 1603 passing tests, 88.3% line coverage
- **v2.2.0** (2025-06-30): Cross-platform production release with CI/CD
- **v2.1.3** (2025-06-30): Code signing integration
- **v2.1.1** (2025-06-30): Production release with critical fixes
- **v2.1.0** (2025-06-30): Interactive drawing widget implementation
- **v2.0.0** (2025-06-29): Major GUI redesign and Qt6 migration

## Related Projects

- [2D Truss Analysis (Python)](https://github.com/blackbird410/2D_Truss_Analysis) - Original Python implementation

---

**Made with dedication for the civil engineering community**
