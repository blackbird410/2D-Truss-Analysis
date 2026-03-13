# Module Structure

This document is a concise module reference. For full architectural detail see the dedicated documents linked in [system-overview.md](system-overview.md).

---

## TrussUtils — `src/utilities/`

Stateless utility functions with no dependencies on other project modules.

| File             | Responsibility                                          |
| ---------------- | ------------------------------------------------------- |
| `math_utils.hpp` | Floating-point comparison, angle/length calculations    |
| `string_utils`   | String formatting helpers used by exporters and logging |

---

## TrussCore — `src/core/`

The domain layer. Contains the structural model, analysis engine, and validator. Has no dependency on any other project layer.

### Model — `src/core/model/`

| Type        | Responsibility                                                                |
| ----------- | ----------------------------------------------------------------------------- |
| `Node`      | 2D node with position, support type, applied load, computed displacement      |
| `Member`    | Truss member connecting two nodes; carries material and section properties    |
| `Load`      | Applied nodal force in X and Y directions                                     |
| `Truss`     | Aggregate owning a collection of nodes and members                            |
| `types.hpp` | Shared value types: `NodeId`, `MemberId`, `Point2D`, `Force2D`, `SupportType` |

### Analysis Engine — `src/core/analysis/`

Implements the direct stiffness method for linear static analysis.

| Component                  | Responsibility                                           |
| -------------------------- | -------------------------------------------------------- |
| `AnalysisOrchestrator`     | Coordinates assembly, solving, and result extraction     |
| `StiffnessAssembler`       | Builds global stiffness matrix from member contributions |
| `BoundaryConditionHandler` | Applies support constraints (pinned, roller-X, roller-Y) |
| `DirectSolver`             | Direct Eigen-based linear solve (LDLT factorization)     |
| `IterativeSolver`          | Iterative fallback solver                                |
| `SolverFactory`            | Selects solver strategy                                  |
| `ILinearSolver`            | Solver interface                                         |

### Validation — `src/core/validation/`

| Component        | Responsibility                                                           |
| ---------------- | ------------------------------------------------------------------------ |
| `TrussValidator` | Checks static determinacy, connectivity, and boundary condition validity |

### Core Interfaces — `src/core/interfaces/`

Read-only view types that cross the facade boundary:

| Interface / Type       | Exposes                                                         |
| ---------------------- | --------------------------------------------------------------- |
| `ITrussView`           | Sequence of `NodeView` and `MemberView` DTOs                    |
| `NodeView`             | id, position, support, load, displacement, reactions            |
| `MemberView`           | id, node ids, properties, axial force, stress, utilization      |
| `IAnalysisResultsView` | Displacements, reactions, member forces, convergence statistics |

---

## TrussInfrastructure — `src/infrastructure/`

Implements I/O and export adapters. Depends on `TrussCore` for domain types.

### Exporters — `src/infrastructure/export/`

All exporters implement a common interface. Supported formats: CSV, JSON, XML, HTML, LaTeX, plain text, TSV. An `ExporterFactory` creates the correct exporter from a format string or file extension.

### File I/O — `src/infrastructure/io/`

JSON and XML serialization/deserialization of `Truss` objects. Uses nlohmann-json and tinyxml2.

### Logging — `src/infrastructure/logging/`

Structured logging subsystem with configurable levels (DEBUG, INFO, WARNING, ERROR).

---

## TrussApplication — `src/application/`

Application services orchestrate domain operations and enforce use-case rules. Depend on `TrussCore` and `TrussInfrastructure`.

| Component                       | Responsibility                                                      |
| ------------------------------- | ------------------------------------------------------------------- |
| `TrussApplicationService`       | CRUD operations on `Truss` objects; delegates to core and I/O       |
| `AnalysisApplicationService`    | Orchestrates analysis execution, caches result handles              |
| `MaterialLibraryService`        | Manages a library of named material and section presets             |
| `ITrussService`                 | Interface for truss lifecycle and editing operations                |
| `IAnalysisService`              | Interface for analysis operations                                   |
| `Result<T>`                     | Discriminated union for operation outcomes (success value or error) |
| `TrussHandle` / `ResultsHandle` | Opaque integer handles identifying live truss and result objects    |

---

## TrussInterface — `src/interface/`

Provides the single integration point for the GUI and CLI layers.

| Component                      | Responsibility                                         |
| ------------------------------ | ------------------------------------------------------ |
| `ITrussAnalysisFacade`         | Unified interface for all GUI/CLI operations           |
| `TrussAnalysisFacade`          | Concrete implementation; composes application services |
| `FacadeTrussServiceAdapter`    | Adapts `ITrussService` for facade consumption          |
| `FacadeAnalysisServiceAdapter` | Adapts `IAnalysisService` for facade consumption       |
| `TrussBuilder`                 | Fluent builder for constructing test/example trusses   |

---

## TrussAnalysisCLI — `src/cli/`

Headless command-line interface. Depends only on `ITrussAnalysisFacade`.

Supported commands: `analyze`, `validate`, `export`, `example`, `help`.

Input formats: JSON, XML, CSV. Output formats: JSON, XML, CSV, TSV, TXT, HTML, LaTeX.

---

## TrussAnalysisGUI — `src/gui/`

Qt6 Widgets interface. See [gui-architecture.md](gui-architecture.md) for the full description.

---

## Test Architecture — `tests/`

```
tests/
├── unit/
│   ├── application/    # TrussApplicationService, AnalysisApplicationService
│   ├── cli/            # CLI command parsing and dispatch
│   ├── core/
│   │   ├── analysis/   # Orchestrator, solvers, stiffness assembler
│   │   ├── assembly/   # TrussAssembler
│   │   ├── model/      # Node, Member, Truss, Load
│   │   └── validation/ # TrussValidator
│   ├── gui/            # Controllers, Qt item models, panels, widgets
│   ├── infrastructure/ # Exporters, file I/O, logging
│   ├── interface/      # Facade adapter tests
│   └── utilities/      # Math and string utilities
├── integration/        # End-to-end workflow tests
├── fixtures/           # Golden master files (CSV, JSON, XML, HTML, LaTeX, TXT)
└── mocks/              # MockTrussAnalysisFacade (GMock)
```

**Test counts and coverage (as of v3.0.0):**

| Metric            | Value |
| ----------------- | ----- |
| Total tests       | 1603  |
| Pass rate         | 100%  |
| Line coverage     | 88.3% |
| Function coverage | 91.9% |
| Branch coverage   | 51.8% |

All five CTest targets pass: `unit_tests`, `unit_tests_gui_widgets`, `integration_tests`, `gui_integration_test`, `test_gtest_integration`.
