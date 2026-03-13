# Core Domain Architecture

## Overview

The Core layer (`TrussCore`) is the innermost layer of the system. It owns the structural domain model, the analysis engine, the validator, and the assembly utilities. It has no dependency on any other project layer. All higher layers depend on `TrussCore`; `TrussCore` depends only on `TrussUtils` and Eigen3.

The Core layer is compiled into the `TrussCore` static library.

---

## Structural Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                         TRUSSCORE                            │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │                   DOMAIN MODEL                         │  │
│  │   Truss    Node    Member    Load    types.hpp         │  │
│  └────────────────────────┬───────────────────────────────┘  │
│                           │ implements                       │
│  ┌────────────────────────▼───────────────────────────────┐  │
│  │               CORE INTERFACES (read-only views)        │  │
│  │   ITrussView    IAnalysisResultsView    TrussDTO       │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │                ASSEMBLY UTILITIES                      │  │
│  │             TrussAssembler  (DTO ↔ Domain)             │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │                   ANALYSIS ENGINE                      │  │
│  │   AnalysisOrchestrator  StiffnessAssembler             │  │
│  │   BoundaryConditionHandler  SolverFactory              │  │
│  │   ILinearSolver  DirectSolver  IterativeSolver         │  │
│  │   AnalysisOptions  AnalysisResults                     │  │
│  └────────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │                   VALIDATION                           │  │
│  │   TrussValidator  ValidationResult  ValidationIssue    │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                              │
                    TrussUtils (math_utils, string_utils)
                    Eigen3
```

---

## Shared Types — `src/core/model/types.hpp`

All primitive value types used throughout the system are defined here.

| Type / Enum          | Kind   | Description                                                             |
| -------------------- | ------ | ----------------------------------------------------------------------- |
| `Real`               | Alias  | `double` — scalar used for all FEA quantities                           |
| `Index`              | Alias  | `std::size_t` — DOF and container offsets                               |
| `NodeId`             | Alias  | `std::uint32_t` — unique identifier for nodes                           |
| `MemberId`           | Alias  | `std::uint32_t` — unique identifier for members                         |
| `LoadId`             | Alias  | `std::uint32_t` — unique identifier for load cases                      |
| `Point2D`            | Struct | 2D coordinate `{x, y}` in metres (X+ rightward, Y+ upward)              |
| `Force2D`            | Struct | 2D force vector `{fx, fy}` in Newtons                                   |
| `SupportType`        | Enum   | `Free`, `Pinned`, `RollerX`, `RollerY`                                  |
| `MaterialProperties` | Struct | `youngModulus`, `density`, `yieldStrength`, `ultimateStrength`, `name`  |
| `SectionProperties`  | Struct | `area`, `momentOfInertia`, `shearArea`, `designation`                   |
| `MemberResults`      | Struct | `axialForce`, `axialStress`, `utilizationRatio`, `inTension`, `yielded` |
| `NodeResults`        | Struct | `displacement` (`Point2D`), `reaction` (`Force2D`)                      |

Eigen type aliases (`Vector2d`, `MatrixXd`, `VectorXd`) are also declared here for consistent use across the Core layer.

---

## Domain Model — `src/core/model/`

### Core Components

| Class / File | Responsibility                                                                                                                                                                                  |
| ------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Node`       | 2D structural joint. Holds position, `SupportType`, applied `Force2D`, DOF indices, computed `NodeResults`. Owns its own `NodeId`.                                                              |
| `Member`     | Structural bar connecting two `Node` objects via `std::shared_ptr`. Holds `MaterialProperties`, `SectionProperties`. Computes length, angle, local/global stiffness matrix, axial force/stress. |
| `Load`       | Named applied force (`Force2D`) with a `LoadType` discriminator. Applied to nodes.                                                                                                              |
| `Truss`      | Aggregate owning nodes as `std::unordered_map<NodeId, NodePtr>` and members as `std::unordered_map<MemberId, MemberPtr>`. Implements `ITrussView`. Manages DOF assignment and boundary queries. |

### Truss Ownership Model

```
Truss
 ├── nodes: unordered_map<NodeId, shared_ptr<Node>>
 └── members: unordered_map<MemberId, shared_ptr<Member>>
                └── startNode: weak_ptr<Node> (shared ownership with Truss)
                └── endNode:   weak_ptr<Node>
```

`Truss` is the sole owner of all `Node` objects. Members hold `shared_ptr<Node>` references to avoid dangling pointers after node removal. Cloning a `Truss` deep-copies all nodes and members.

### `Truss` — ITrussView Implementation

`Truss` implements `ITrussView` to provide a read-only projection of its state to the Infrastructure layer. It returns lightweight `NodeView` and `MemberView` value structs rather than exposing internal node/member pointers.

---

## Core Interfaces — `src/core/interfaces/`

These interfaces are the only types the Infrastructure layer uses to read domain data. They enforce Dependency Inversion: Infrastructure depends on these abstractions, not on concrete `Truss` or `AnalysisResults` classes.

### `ITrussView`

| Method                 | Returns                   | Description                                |
| ---------------------- | ------------------------- | ------------------------------------------ |
| `getName()`            | `const std::string&`      | Truss display name                         |
| `getNodeViews()`       | `std::vector<NodeView>`   | Snapshot of all node data                  |
| `getMemberViews()`     | `std::vector<MemberView>` | Snapshot of all member data                |
| `getNodeCount()`       | `size_t`                  | Number of nodes                            |
| `getMemberCount()`     | `size_t`                  | Number of members                          |
| `getTotalDofs()`       | `size_t`                  | Total system DOFs (2 × node count)         |
| `getFreeDofs()`        | `size_t`                  | Unconstrained DOFs                         |
| `getConstrainedDofs()` | `size_t`                  | Constrained DOFs (from support conditions) |

### `IAnalysisResultsView`

| Method                   | Returns                       | Description                                |
| ------------------------ | ----------------------------- | ------------------------------------------ |
| `getDisplacements()`     | `const vector<Real>&`         | Global nodal displacement vector (m)       |
| `getReactions()`         | `const vector<Real>&`         | Support reaction forces (N)                |
| `getMemberForces()`      | `const vector<Real>&`         | Axial forces per member (N, +ve = tension) |
| `getMemberStresses()`    | `const vector<Real>&`         | Axial stresses per member (Pa)             |
| `getUtilizationRatios()` | `const vector<Real>&`         | σ / f_y per member (dimensionless)         |
| `getStiffnessMatrix()`   | `const vector<vector<Real>>&` | Assembled global stiffness matrix (N/m)    |
| `hasConverged()`         | `bool`                        | Whether the linear solve converged         |
| `getIterations()`        | `int`                         | Solver iteration count                     |
| `getResidualNorm()`      | `Real`                        | Final residual norm                        |
| `getConditionNumber()`   | `Real`                        | Stiffness matrix condition number          |

### View DTOs

| Type         | Purpose                                                                            |
| ------------ | ---------------------------------------------------------------------------------- |
| `NodeView`   | Plain value struct: id, x, y, support, applied forces, displacements, reactions    |
| `MemberView` | Plain value struct: id, node ids, material/section props, analysis results         |
| `TrussDTO`   | Serialization bridge for the full truss; contains `NodeDTO` and `MemberDTO` arrays |

---

## Assembly Utilities — `src/core/assembly/`

| Class            | Responsibility                                                                                                                                                                |
| ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `TrussAssembler` | Static utility. Converts `TrussDTO` → `Truss` (domain object construction from file I/O DTOs) and `ITrussView` → `TrussDTO` (domain to serialization form). Non-instantiable. |

The assembler enforces a clean boundary: Infrastructure I/O readers produce DTOs; `TrussAssembler` promotes those DTOs into fully-wired domain objects. Infrastructure never directly constructs `Node` or `Member` instances.

---

## Interaction with Other Layers

| Layer           | What it uses from Core                                            |
| --------------- | ----------------------------------------------------------------- |
| Analysis Engine | `Truss` (direct access), `types.hpp` primitives                   |
| Validation      | `Truss` (direct access)                                           |
| Infrastructure  | `ITrussView`, `IAnalysisResultsView`, `TrussDTO` — never `Truss*` |
| Application     | `Truss` (direct mutable access via `getTrussMutable()`)           |
| Interface / GUI | `ITrussView`, `IAnalysisResultsView` through facade               |

---

## Key Design Constraints

- `TrussCore` has no dependency on `TrussInfrastructure`, `TrussApplication`, or `TrussInterface`.
- `Truss` implements `ITrussView` directly — no separate adapter required.
- `AnalysisResults` implements `IAnalysisResultsView` directly.
- `NodeView` and `MemberView` are plain value structs (no virtual methods, no inheritance); they are safe to copy across layer boundaries.
- `SupportType::Pinned` always constrains both X and Y translations. Directional-only constraints are modeled as `RollerX` or `RollerY`. There is no `PinnedX` or `PinnedY` — this is a deliberate mechanics constraint.
- All Eigen types are local to the Core and Analysis layers; they never appear in public interface headers consumed by GUI or CLI.
