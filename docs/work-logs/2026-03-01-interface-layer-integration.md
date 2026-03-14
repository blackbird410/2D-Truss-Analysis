# Interface Layer Integration: Facade, Builder, CLI and GUI Migration

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 1, 2026  
**Status:** ✅ Complete  
**Scope:** Introduction of the interface layer, full CLI and GUI service decoupling via facade, and consolidation of cross-layer API contracts

---

## Context

The application layer previously exposed concrete service implementations directly to the CLI and GUI components, creating tight coupling that made independent testing and substitution impractical. This work introduces a dedicated interface layer built around `TrussAnalysisFacade` and `TrussBuilder`, migrates both entry points to consume services exclusively through this facade, and establishes `ITrussAnalysisFacade` as the abstract contract governing adapter-layer dependencies. Supporting structural work reduces cross-layer duplication and resolves naming ambiguities in the application contract.

---

## Scope of Work

**Interface Layer**

- Implemented `TrussAnalysisFacade` as a unified facade over truss analysis workflows
- Implemented `TrussBuilder` as a fluent builder for programmatic truss construction
- Introduced `ITrussAnalysisFacade` as an abstract interface over the concrete facade
- Expanded the facade to satisfy `ITrussService` and `IAnalysisService` contracts
- Implemented `FacadeAnalysisServiceAdapter` and `FacadeTrussServiceAdapter`

**API Consolidation**

- Consolidated duplicated `ExportFormat` enum definitions across infrastructure, application, CLI, and interface layers into a single public API
- Disambiguated handle validation methods by entity type in the application interface contract

**CLI Migration**

- Refactored all CLI commands (`AnalyzeCommand`, `ExampleCommand`, `ExportCommand`, `ValidateCommand`) and the CLI entry point to depend exclusively on `ITrussAnalysisFacade`, removing all direct compile-time dependencies on the concrete `TrussAnalysisFacade` implementation

**GUI Migration**

- Decoupled GUI widgets and the application entry point from all concrete service classes
- Wired service adapter injection through the facade adapter layer

**Build**

- Added `TrussInterface` as a static library CMake target
- Linked `TrussInterface` to CLI, GUI, and integration test targets
- Included facade adapter sources in the `TrussInterface` target
- Registered interface layer and facade adapter test targets in the CMake configuration
- Added public API header directories to relevant target include paths
- Standardised CTest suite aliases to snake_case; registered a lowercase `unit_tests` alias

**Style**

- Applied clang-format to all modified C++ source and header files

---

## Technical Changes

### Interface Abstraction

`ITrussAnalysisFacade` was introduced as a pure abstract interface, replacing all direct references to the concrete `TrussAnalysisFacade` class within the adapter layer. Both `FacadeAnalysisServiceAdapter` and `FacadeTrussServiceAdapter` now depend exclusively on this interface, enabling mock substitution without concrete class knowledge.

`TrussAnalysisFacade` was extended to satisfy `ITrussService` and `IAnalysisService` contracts, allowing it to be injected wherever either service interface is expected.

`ITrussAnalysisFacade` was further extended to declare the high-level workflow methods — `analyzeFromFile`, `analyzeInteractive`, `validateFromFile`, and the simplified `exportResults` overloads — as pure virtuals, and `AnalysisWorkflowResult` was relocated from the concrete facade header to this interface. This ensures that all consumers of the workflow API depend solely on the abstract interface without requiring the concrete facade header.

### ExportFormat Centralisation

Layer-local `ExportFormat` enum definitions in the infrastructure, application, CLI, and interface layers were removed and replaced with a single public API definition. All consumers—including test mocks—were updated to reference this location. Public API header directories were added to the affected CMake targets to make the centralised definition resolvable without path workarounds.

### Handle Validation Disambiguation

Handle validation methods in the application layer were renamed to distinguish between truss and analysis entity types. The rename was propagated consistently across the application, interface, mock, integration, and unit test layers to eliminate the prior naming collision.

### CLI and GUI Decoupling

All CLI command handlers and the CLI entry point were refactored to depend exclusively on `ITrussAnalysisFacade`. Concrete facade instantiation is now confined to the composition root, eliminating direct compile-time dependencies on `TrussAnalysisFacade` within the CLI layer. In the GUI layer, widget constructors and the application entry point were updated to accept abstract service interface parameters, removing all compile-time dependencies on concrete service types.

---

## Testing and Validation

- Added comprehensive unit tests for `TrussAnalysisFacade` and `TrussBuilder`, including edge cases for facade and builder workflows.
- Added unit tests for `FacadeAnalysisServiceAdapter` and `FacadeTrussServiceAdapter`.
- Added end-to-end integration tests for facade adapter behaviour.
- Introduced a GoogleMock class for `ITrussAnalysisFacade`; replaced all unsafe cast-based mock injection in adapter tests with typed interface injection.
- Replaced weak geometric bounds assertions with exact geometry assertions across adapter tests.
- Replaced all silently skipped test cases (`GTEST_SKIP`) with strict, deterministic assertions; no suppressed failures remain in the adapter or CLI test suites.
- Added error-forwarding coverage to the truss adapter test suite.
- Updated all CLI command unit tests and the CLI workflow integration suite to inject `TrussAnalysisFacade` via the `ITrussAnalysisFacade` interface.
- Updated all mock, unit, and integration test layers for `ExportFormat` API migration and handle validation rename.
- Updated coverage reports to reflect the interface layer addition.
- Build verified stable with all new library, adapter, and test targets correctly registered, linked, and passing.

---

## Outcome

The interface layer is fully operational and integrated across both the CLI and GUI entry points. Neither entry point retains any direct dependency on a concrete facade or service implementation; all service interactions are mediated through `ITrussAnalysisFacade`, `ITrussService`, and `IAnalysisService`. Concrete instantiation is confined to the composition root. The facade adapters now depend strictly on abstractions, fully aligning with the Dependency Inversion Principle. The `ExportFormat` enum is defined in a single public API location, and handle validation method names are unambiguous across all layers. No breaking changes were introduced to the external public API surface.
