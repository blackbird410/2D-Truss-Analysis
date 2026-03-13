# System Overview

## Overview

2D Truss Analysis is a professional structural analysis application for planar truss structures. It implements the direct stiffness method to compute nodal displacements, member forces, and support reactions. The application targets civil engineering students and practitioners working with 2D truss structures. It ships as two executables sharing a common library stack: a Qt6 Widgets GUI and a headless CLI.

---

## Capabilities

- Interactive 2D canvas-based model construction (node placement, member drawing, load/support assignment)
- Direct stiffness method solver with linear static analysis
- Structural validation (static determinacy, connectivity, boundary conditions, kinematic stability)
- Multi-format export: CSV, TSV, JSON, XML, HTML, LaTeX, plain text
- Qt6 Widgets GUI (`truss-analysis`) and headless CLI (`truss-analysis-cli`)
- Dark and light QSS themes with `QSettings` persistence

---

## Structural Diagram

Dependencies flow strictly downward. No lower layer references a higher one.

```
┌────────────────────────────────────────────────────────────────────┐
│                      PRESENTATION LAYER                            │
│   TrussAnalysisGUI (Qt6 Widgets MVC)                               │
│   TrussAnalysisCLI (Command pattern, ICommand dispatch)            │
└────────────────────────────┬───────────────────────────────────────┘
                             │ ITrussAnalysisFacade (sole coupling point)
┌────────────────────────────▼───────────────────────────────────────┐
│                      INTERFACE LAYER                               │
│   TrussAnalysisFacade   FacadeTrussServiceAdapter                  │
│   ITrussAnalysisFacade  FacadeAnalysisServiceAdapter               │
│   TrussBuilder                                                     │
└────────────────────────────┬───────────────────────────────────────┘
                             │ ITrussService / IAnalysisService
┌────────────────────────────▼───────────────────────────────────────┐
│                    APPLICATION LAYER                               │
│   TrussApplicationService   AnalysisApplicationService             │
│   MaterialLibraryService    Result<T>                              │
│   TrussHandle               ResultsHandle                          │
└──────────┬─────────────────────────────────────┬───────────────────┘
           │ domain model types                  │ ITrussView / IAnalysisResultsView
┌──────────▼─────────────────┐   ┌───────────────▼───────────────────┐
│       CORE LAYER           │   │    INFRASTRUCTURE LAYER           │
│  Model: Node Member Truss  │   │  ExporterFactory  IResultsExporter│
│  Analysis: Orchestrator    │   │  FileIOFactory    ITrussReader    │
│  Validation: TrussValidator│   │  LoggerFactory    ILogger         │
│  Assembly:  TrussAssembler │   │  ConsoleOutputAdapter             │
└──────────┬─────────────────┘   └───────────────────────────────────┘
           │
┌──────────▼─────────────────┐
│      UTILITIES LAYER       │
│   math_utils   string_utils│
└────────────────────────────┘
```

---

## CMake Targets

| Target                | Type           | Source                | Executable / Binary  |
| --------------------- | -------------- | --------------------- | -------------------- |
| `TrussUtils`          | Static library | `src/utilities/`      | —                    |
| `TrussCore`           | Static library | `src/core/`           | —                    |
| `TrussInfrastructure` | Static library | `src/infrastructure/` | —                    |
| `TrussApplication`    | Static library | `src/application/`    | —                    |
| `TrussInterface`      | Static library | `src/interface/`      | —                    |
| `TrussAnalysisCLI`    | Executable     | `src/cli/`            | `truss-analysis-cli` |
| `TrussAnalysisGUI`    | Executable     | `src/gui/`            | `truss-analysis`     |

---

## Technology Stack

| Component       | Technology                              |
| --------------- | --------------------------------------- |
| Language        | C++20                                   |
| GUI framework   | Qt 6 (Core, Widgets, GUI)               |
| Linear algebra  | Eigen3                                  |
| Serialization   | nlohmann-json, tinyxml2                 |
| Testing         | Google Test + GoogleMock                |
| Build system    | CMake 3.20+, Ninja, production Makefile |
| Coverage        | lcov / genhtml                          |
| Static analysis | clang-tidy                              |
| Formatting      | clang-format                            |
| Platform        | Linux (Ubuntu 22.04+, Fedora, Arch)     |
| Dependencies    | Managed via vcpkg (manifest mode)       |

---

## Interface Boundary

All GUI and CLI interaction with the domain flows through a single interface:

```
src/interface/itruss_analysis_facade.hpp  →  ITrussAnalysisFacade
```

No widget, panel, or CLI command class holds a direct reference to any domain or application class. All operations are performed exclusively through `ITrussAnalysisFacade` or the sub-interfaces (`ITrussService`, `IAnalysisService`) it inherits.

---

## Key Design Rules

- **Strict downward dependency** — no lower layer imports from a higher layer.
- **Single facade coupling point** — GUI and CLI depend only on `ITrussAnalysisFacade`.
- **Read-only view isolation** — infrastructure exporters and Qt item models access domain data only through `ITrussView` and `IAnalysisResultsView`; they never hold a concrete `Truss*` or `AnalysisResults*`.
- **Opaque handles** — `TrussHandle` and `ResultsHandle` are integer aliases; callers never dereference domain objects directly.
- **Validation before solving** — `TrussValidator::validate()` must return `isValid() == true` before `AnalysisOrchestrator` is invoked.
- **Error propagation without exceptions** — all application-layer operations return `Result<T>`; exceptions do not cross layer boundaries.

---

## Layer Documentation Index

| Document                                                             | Covers                                         |
| -------------------------------------------------------------------- | ---------------------------------------------- |
| [system-overview.md](system-overview.md)                             | This document — full system structure          |
| [core-domain-architecture.md](core-domain-architecture.md)           | Domain model, assembly, core interfaces        |
| [analysis-engine-architecture.md](analysis-engine-architecture.md)   | Solvers, stiffness assembly, orchestration     |
| [validation-layer-architecture.md](validation-layer-architecture.md) | Structural validation rules                    |
| [facade-architecture.md](facade-architecture.md)                     | Interface/facade layer, adapters, builder      |
| [gui-architecture.md](gui-architecture.md)                           | Qt6 Widgets MVC — controllers, panels, signals |
| [testing-architecture.md](testing-architecture.md)                   | Test structure, mocking strategy, coverage     |
