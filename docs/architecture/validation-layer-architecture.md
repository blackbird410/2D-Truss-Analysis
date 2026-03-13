# Validation Layer Architecture

## Overview

The Validation layer resides in `src/core/validation/` and is compiled into the `TrussCore` static library. It provides comprehensive pre-analysis verification of a `Truss` structural model, separating validation logic entirely from the domain entities following the Single Responsibility Principle.

`TrussValidator` is the sole validation class. It is a stateless utility class — all methods are `static`. Validation produces a `ValidationResult` containing zero or more typed `ValidationIssue` records.

---

## Structural Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                    VALIDATION LAYER                          │
│                                                              │
│                     TrussValidator                           │
│                   (all methods static)                        │
│                                                              │
│   validate()                   isValid()                     │
│      │                                                        │
│      ├─ validateStructuralCompleteness()                      │
│      ├─ validateGeometry()                                    │
│      ├─ validateMaterials()                                   │
│      ├─ validateBoundaryConditions()                          │
│      ├─ validateStaticDeterminacy()                           │
│      ├─ validateKinematicStability()                          │
│      ├─ validateLoads()                                       │
│      └─ validateConnectivity()                               │
│                │                                             │
│                ▼                                             │
│          ValidationResult                                    │
│           ├── ValidationIssue (severity=Info)                │
│           ├── ValidationIssue (severity=Warning)             │
│           ├── ValidationIssue (severity=Error)               │
│           └── ValidationIssue (severity=Fatal)               │
└──────────────────────────────────────────────────────────────┘
                     │
               const Truss&
```

---

## Core Components

| Component            | Responsibility                                                                                                                                                                |
| -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `TrussValidator`     | Stateless utility; runs all validation checks and accumulates issues. Entry points: `validate()` (full report) and `isValid()` (quick pass/fail check).                       |
| `ValidationResult`   | Collection of `ValidationIssue` records. Provides `isValid()`, `hasErrors()`, `hasFatal()`, `hasWarnings()`, `getIssues()`, `getIssuesBySeverity()`, `getIssuesByCategory()`. |
| `ValidationIssue`    | Single diagnostic record with severity, category, human-readable message, technical detail, and lists of affected node/member IDs.                                            |
| `ValidationSeverity` | Enum: `Info`, `Warning`, `Error`, `Fatal`.                                                                                                                                    |

---

## Validation Checks

`TrussValidator::validate()` executes all eight checks in sequence. Each check populates the shared `ValidationResult` with zero or more issues.

| Check Method                     | Category       | What It Validates                                                                              |
| -------------------------------- | -------------- | ---------------------------------------------------------------------------------------------- |
| `validateStructuralCompleteness` | `Completeness` | Minimum node count (≥ 2), minimum member count (≥ 1), absence of null node/member pointers     |
| `validateGeometry`               | `Geometry`     | Non-zero member lengths, no coincident nodes, finite coordinate values                         |
| `validateMaterials`              | `Material`     | Positive Young's modulus, positive cross-sectional area, positive density for all members      |
| `validateBoundaryConditions`     | `Boundary`     | At least one support constraint present; adequate reaction count to prevent rigid-body motion  |
| `validateStaticDeterminacy`      | `Determinacy`  | Static determinacy check: 2n = m + r (nodes, members, reactions)                               |
| `validateKinematicStability`     | `Stability`    | Detects kinematically unstable configurations (prevents infinite displacements at solver time) |
| `validateLoads`                  | `Loads`        | At least one non-zero applied load; load application points correspond to existing nodes       |
| `validateConnectivity`           | `Connectivity` | All nodes are reachable via members (no isolated nodes); no disconnected sub-structures        |

---

## `ValidationIssue`

| Field             | Type                    | Description                                                          |
| ----------------- | ----------------------- | -------------------------------------------------------------------- |
| `severity`        | `ValidationSeverity`    | `Info` / `Warning` / `Error` / `Fatal`                               |
| `category`        | `std::string`           | Domain of the check (e.g., `"Geometry"`, `"Boundary"`, `"Material"`) |
| `message`         | `std::string`           | Human-readable description for display in GUI or CLI output          |
| `technicalDetail` | `std::string`           | Engineering explanation (optional; may be empty)                     |
| `affectedNodes`   | `std::vector<NodeId>`   | Node IDs implicated by the issue (may be empty)                      |
| `affectedMembers` | `std::vector<MemberId>` | Member IDs implicated by the issue (may be empty)                    |

---

## Severity Semantics

| Severity  | Meaning                                                              | Blocks Analysis? |
| --------- | -------------------------------------------------------------------- | ---------------- |
| `Info`    | Informational note; no action required                               | No               |
| `Warning` | Potential issue; analysis may proceed but results should be reviewed | No               |
| `Error`   | Critical issue; analysis must not proceed                            | **Yes**          |
| `Fatal`   | Structural impossibility or data corruption                          | **Yes**          |

`ValidationResult::isValid()` returns `true` only when there are no `Error` or `Fatal` issues.

---

## Data Flow

```
Application Layer
     │
     │  ITrussService::validateTruss(handle)
     ▼
TrussApplicationService
     │  TrussValidator::validate(truss)
     ▼
TrussValidator
     │  returns ValidationResult
     ▼
Application Layer
     │  wraps in Result<ValidationResult>
     ▼
ITrussAnalysisFacade  →  GUI / CLI consumers
```

The GUI's `ValidationListModel` consumes the `ValidationResult::getIssues()` vector directly. Each `ValidationIssue` maps to one row in the inspector validation summary panel.

---

## Interaction with Other Layers

| Layer           | Interaction                                                                                                                      |
| --------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Core Model      | Reads `const Truss&` — no mutation; uses `Node`, `Member`, and DOF data                                                          |
| Analysis Engine | Must succeed before `AnalysisOrchestrator::analyze()` is invoked                                                                 |
| Application     | `TrussApplicationService::validateTruss()` calls `TrussValidator::validate()` and wraps the result in `Result<ValidationResult>` |
| GUI             | `ValidationListModel` renders `ValidationIssue` records; `InspectorController` triggers validation before analysis               |
| CLI             | `ValidateCommand` calls `ITrussAnalysisFacade::validateFromFile()` and displays issues via `ConsolePresenter`                    |

---

## Key Design Constraints

- `TrussValidator` is stateless — all methods are `static`. No instances are required.
- Validation runs on a fully constructed `Truss` before the solver is invoked; it is never called mid-analysis.
- The Application layer (`TrussApplicationService`) is the sole caller of `TrussValidator`; Domain objects do not self-validate.
- `ValidationResult::isValid() == false` results in the Application layer returning an error `Result<T>`, preventing `AnalysisOrchestrator` from running.
- Individual check methods are `public static` and may be called independently (e.g., for targeted checks during interactive model editing).
