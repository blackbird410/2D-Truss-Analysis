# Facade Architecture

## Overview

The Interface layer (`TrussInterface`, compiled to `src/interface/`) is the single integration point for all presentation-layer consumers — both the Qt6 GUI and the headless CLI. It provides the `ITrussAnalysisFacade` interface, the `TrussAnalysisFacade` concrete implementation, two service adapters, and the `TrussBuilder` fluent construction utility.

The Application layer (`TrussApplication`, `src/application/`) sits directly below. It owns the use-case service implementations: `TrussApplicationService`, `AnalysisApplicationService`, and `MaterialLibraryService`.

---

## Structural Diagram

```
┌──────────────────────────────────────────────────────────────┐
│           GUI (Qt6 Widgets)    CLI (ICommand dispatch)       │
└──────────────────────┬───────────────────────────────────────┘
                       │ ITrussAnalysisFacade*
┌──────────────────────▼───────────────────────────────────────┐
│                    INTERFACE LAYER                           │
│                                                              │
│   ITrussAnalysisFacade   ←  extends ITrussService            │
│         (interface)         extends IAnalysisService         │
│              ▲                                               │
│              │ implements                                    │
│   TrussAnalysisFacade                                        │
│         (concrete)                                           │
│    ┌───────────────────┐                                     │
│    │ FacadeTrussService│   FacadeAnalysisServiceAdapter      │
│    │ Adapter           │   (implements IAnalysisService)     │
│    │ (implements       │                                     │
│    │  ITrussService)   │                                     │
│    └──────────┬────────┘                                     │
│               │ delegates to facade public API               │
│   TrussBuilder (fluent construction helper)                  │
└──────────────────────────────────────────────────────────────┘
                       │ ITrussService / IAnalysisService
┌──────────────────────▼───────────────────────────────────────┐
│                  APPLICATION LAYER                           │
│                                                              │
│   ITrussService ◄── TrussApplicationService                  │
│   IAnalysisService ◄── AnalysisApplicationService            │
│   IMaterialLibrary ◄── MaterialLibraryService                │
│   Result<T>    TrussHandle    ResultsHandle                  │
│   MaterialSpec   SectionSpec  (truss_edit_dtos.hpp)          │
└──────────────────────────────────────────────────────────────┘
```

---

## Interface Layer Components

### `ITrussAnalysisFacade`

The unified public API. Inherits from both `ITrussService` and `IAnalysisService` (diamond inheritance, resolved explicitly with a single `clearAll()` declaration). All methods that cross layer boundaries are declared as pure virtuals here.

#### High-Level Workflow Methods

| Method                                          | Description                                                                      |
| ----------------------------------------------- | -------------------------------------------------------------------------------- |
| `analyzeFromFile(filepath, options)`            | Complete workflow: load → validate → analyze. Returns `AnalysisWorkflowResult`.  |
| `analyzeInteractive(builder, options)`          | Complete workflow: build → validate → analyze. Returns `AnalysisWorkflowResult`. |
| `validateFromFile(filepath)`                    | Load → validate (no analysis). Returns `ValidationResult`.                       |
| `exportResults(handle, format, filepath, opts)` | Export with explicit format enum.                                                |
| `exportResults(handle, filepath, opts)`         | Export with format auto-detected from file extension.                            |

#### Inherited from `ITrussService`

| Method                                   | Description                                                 |
| ---------------------------------------- | ----------------------------------------------------------- |
| `createTruss(name)`                      | Create an empty named truss. Returns `Result<TrussHandle>`. |
| `loadTruss(filepath)`                    | Load from JSON or XML file. Returns `Result<TrussHandle>`.  |
| `saveTruss(handle, filepath, overwrite)` | Save to JSON or XML file. Returns `Result<bool>`.           |
| `clearTruss(handle)`                     | Dispose a truss by handle.                                  |
| `isValidTrussHandle(handle)`             | Check handle liveness.                                      |
| `getTrussView(handle)`                   | Read-only `ITrussView&` for the truss.                      |
| `getTrussMutable(handle)`                | Mutable `Truss&` (Application layer only).                  |
| `validateTruss(handle)`                  | Returns `Result<ValidationResult>`.                         |
| `addNode(handle, position, support)`     | Adds a node. Returns `Result<NodeId>`.                      |
| `addMember(handle, n1, n2, mat, sec)`    | Adds a member. Returns `Result<MemberId>`.                  |
| `removeNode(handle, nodeId)`             | Removes node and incident members.                          |
| `removeMember(handle, memberId)`         | Removes a member.                                           |
| `setNodeSupport(handle, nodeId, type)`   | Changes support condition.                                  |
| `applyNodeLoad(handle, nodeId, force)`   | Applies or replaces a nodal force.                          |
| `clearNodeLoad(handle, nodeId)`          | Removes the applied load from a node.                       |
| `updateNode(handle, nodeId, dto)`        | Updates node position/support.                              |
| `updateMember(handle, memberId, dto)`    | Updates member material/section properties.                 |

#### Inherited from `IAnalysisService`

| Method                                           | Description                                           |
| ------------------------------------------------ | ----------------------------------------------------- |
| `analyze(truss, options)`                        | Run analysis. Returns `Result<ResultsHandle>`.        |
| `getResultsView(handle)`                         | Read-only `IAnalysisResultsView&` for cached results. |
| `exportResults(handle, format, fp, truss, opts)` | Export with explicit format and truss context.        |
| `exportResults(handle, fp, truss, opts)`         | Export with auto-detected format and truss context.   |
| `clearResults(handle)`                           | Disposes a results object by handle.                  |
| `isValidResultsHandle(handle)`                   | Check results handle liveness.                        |

### `AnalysisWorkflowResult`

Return type for the two high-level workflow methods. Defined on the interface, not the concrete class, so all consumers can use it without depending on `TrussAnalysisFacade`.

| Field           | Type            | Description                                   |
| --------------- | --------------- | --------------------------------------------- |
| `success`       | `bool`          | Whether the workflow completed successfully   |
| `errorMessage`  | `std::string`   | Human-readable error description (on failure) |
| `trussHandle`   | `TrussHandle`   | Handle to the loaded/built truss (on success) |
| `resultsHandle` | `ResultsHandle` | Handle to the analysis results (on success)   |

Factory methods: `AnalysisWorkflowResult::Success(th, rh)` and `AnalysisWorkflowResult::Failure(error)`.

---

### `TrussAnalysisFacade`

The concrete implementation of `ITrussAnalysisFacade`. Owns both application service instances internally.

- Owns `TrussApplicationService` (via composition)
- Owns `AnalysisApplicationService` (via composition)
- The adapters (`FacadeTrussServiceAdapter`, `FacadeAnalysisServiceAdapter`) delegate to facade public methods — no `friend` declarations

### Service Adapters

| Adapter                        | Implements         | Delegates to                                                 |
| ------------------------------ | ------------------ | ------------------------------------------------------------ |
| `FacadeTrussServiceAdapter`    | `ITrussService`    | `TrussAnalysisFacade` public API via `ITrussAnalysisFacade&` |
| `FacadeAnalysisServiceAdapter` | `IAnalysisService` | `TrussAnalysisFacade` public API via `ITrussAnalysisFacade&` |

These adapters exist to give GUI controllers a focused `ITrussService` or `IAnalysisService` reference without requiring them to hold the full `ITrussAnalysisFacade*`. They satisfy DIP without friend access.

### `TrussBuilder`

Fluent builder for programmatic truss construction. Used by `analyzeInteractive()` and by example/test code.

| Method                                     | Description                                  |
| ------------------------------------------ | -------------------------------------------- |
| `addNode(x, y, support)`                   | Appends a node; returns `*this` for chaining |
| `addMember(n1id, n2id, material, section)` | Appends a member by node IDs                 |
| `applyForce(nodeId, fx, fy)`               | Applies a force to a node                    |
| `build()`                                  | Returns a fully wired `shared_ptr<Truss>`    |

---

## Application Layer Components

### `TrussApplicationService` — implements `ITrussService`

Manages a `std::unordered_map<TrussHandle, shared_ptr<Truss>>` keyed by opaque handles.

- Handle allocation: monotonically increasing counter.
- File loading: delegates to `FileIOFactory` → reader (JSON/XML) → `TrussAssembler::assembleTruss()`.
- File saving: delegates to `TrussAssembler::createDTO()` → writer (JSON/XML) via `FileIOFactory`.
- Editing: directly mutates the `Truss` in the map (via `getTrussMutable()`).

### `AnalysisApplicationService` — implements `IAnalysisService`

Manages a `std::unordered_map<ResultsHandle, AnalysisResults>` keyed by opaque handles.

- Delegates analysis to `AnalysisOrchestrator::analyze()`.
- Caches `AnalysisResults` under a `ResultsHandle`.
- Delegates export to `ExporterFactory` → `IResultsExporter::exportResults()`.

### `MaterialLibraryService` — implements `IMaterialLibrary`

Maintains named presets for `MaterialProperties` and `SectionProperties`. Provides predefined entries for Steel, Aluminum, and Concrete, plus custom registration.

### `Result<T>`

Monadic error type used for all application-layer operation outcomes.

```cpp
Result<TrussHandle> r = facade.createTruss("Bridge");
if (r) {
    TrussHandle h = r.value;
}
// else: r.errorMessage contains the failure reason
```

---

## Data Flow

```
CLI Command / GUI Controller
       │
       │  ITrussAnalysisFacade::analyzeFromFile(path, opts)
       ▼
TrussAnalysisFacade
       ├─ TrussApplicationService::loadTruss(path)
       │       └─ FileIOFactory → reader → TrussAssembler → Truss
       │
       ├─ TrussApplicationService::validateTruss(handle)
       │       └─ TrussValidator::validate(truss) → ValidationResult
       │
       ├─ AnalysisApplicationService::analyze(truss, opts)
       │       └─ AnalysisOrchestrator::analyze(truss, opts) → AnalysisResults
       │              (cached under ResultsHandle)
       │
       └─ return AnalysisWorkflowResult{trussHandle, resultsHandle}
```

---

## CLI Layer

The CLI layer (`src/cli/`) is compiled as part of the `TrussAnalysisCLI` executable. It depends only on `ITrussAnalysisFacade`.

| Component          | Responsibility                                                                      |
| ------------------ | ----------------------------------------------------------------------------------- |
| `ICommand`         | Interface: `execute()`, `getName()`, `getDescription()`                             |
| `AnalyzeCommand`   | Implements `ICommand`. Calls `analyzeFromFile()` then optionally `exportResults()`  |
| `ValidateCommand`  | Implements `ICommand`. Calls `validateFromFile()` and displays issues               |
| `ExportCommand`    | Implements `ICommand`. Calls `exportResults()` on an existing results handle        |
| `ExampleCommand`   | Implements `ICommand`. Builds a predefined truss via `TrussBuilder` and analyzes    |
| `HelpCommand`      | Implements `ICommand`. Displays usage text                                          |
| `ArgumentParser`   | Parses `argc/argv` into command + option structures                                 |
| `ConsolePresenter` | Formats and prints analysis results and validation issues to stdout with ANSI color |

---

## Interaction with Other Layers

| Layer          | Interaction                                                                                              |
| -------------- | -------------------------------------------------------------------------------------------------------- |
| GUI            | `MainWindowController` holds `ITrussAnalysisFacade*`; sub-controllers receive focused adapter references |
| CLI Commands   | Each command holds `ITrussAnalysisFacade&` injected at construction                                      |
| Application    | `TrussAnalysisFacade` owns and delegates to application service instances                                |
| Core           | Application services use `Truss` directly; facade never touches core types                               |
| Infrastructure | `AnalysisApplicationService` delegates export to `ExporterFactory`                                       |

---

## Key Design Constraints

- GUI and CLI couple only to `ITrussAnalysisFacade`; no presentation-layer class names a concrete service or domain type.
- `AnalysisWorkflowResult` is declared on the interface, not the concrete class, so consumers never need to `#include` `truss_analysis_facade.hpp`.
- `TrussHandle` and `ResultsHandle` are opaque integer aliases. Callers cannot dereference or iterate internal service maps.
- `Result<T>` is the sole error-propagation mechanism across the application and interface layers. Exceptions do not escape the Core layer.
- Adapter delegation uses only the public API of `ITrussAnalysisFacade`; no `friend` declarations are required.
- `TrussBuilder` is part of the Interface layer (not Core) because it holds construction-ordering state that is not a domain concept.
