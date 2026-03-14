# Analysis Engine Architecture

## Overview

The Analysis Engine resides in `src/core/analysis/` and is compiled into the `TrussCore` static library. It implements the direct stiffness method for linear static analysis of 2D truss structures. The engine is fully decomposed: each algorithmic concern is encapsulated in a dedicated, stateless class. The sole coordination point is `AnalysisOrchestrator`.

The engine operates on a `const Truss&` snapshot and produces a populated `AnalysisResults` value object.

---

## Structural Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    ANALYSIS ENGINE                              │
│                                                                 │
│              AnalysisOrchestrator                               │
│             (coordinates full workflow)                         │
│         ┌──────────┬──────────┬───────────────┐                 │
│         │          │          │               │                 │
│ StiffnessAssembler │ BoundaryConditionHandler │ SolverFactory   │
│         │          │          │        ┌──────┴──────┐          │
│         │          │          │        │             │          │
│         │          │          │    DirectSolver IterativeSolver │
│         │          │          │     (LDLT)    (Conj. Gradient)  │
│         │          │          │        └──────┬──────┘          │
│         │          │          │         ILinearSolver           │
│         │          │          │               │                 │
└─────────┼──────────┼──────────┼───────────────┼─────────────────┘
          │          │          │               │
          ▼          ▼          ▼               ▼
    const Truss&                AnalysisResults
                            (implements IAnalysisResultsView)
```

---

## Core Components

| Component                  | Responsibility                                                                                                                                                |
| -------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `AnalysisOrchestrator`     | Executes DOF assignment → stiffness assembly → BC application → solve → post-processing. Owns no persistent state; all work done in `analyze()`.              |
| `AnalysisResults`          | Value object aggregating all analysis outputs. Implements `IAnalysisResultsView`.                                                                             |
| `AnalysisOptions`          | Configuration value struct passed to `analyze()`. Controls solver selection, tolerance, verbosity.                                                            |
| `StiffnessAssembler`       | Builds the global stiffness matrix `K` (`totalDofs × totalDofs`) by accumulating each member's 4×4 local stiffness contribution using its global DOF indices. |
| `BoundaryConditionHandler` | Identifies free and constrained DOFs from node support types. Extracts the reduced system `K_ff` and `F_f`. Expands solution back to full DOF vector.         |
| `ILinearSolver`            | Abstract interface for `Ax = b`. Every concrete solver implements `solve(MatrixXd, VectorXd) → VectorXd`.                                                     |
| `DirectSolver`             | Implements `ILinearSolver` using Eigen's LDLT symmetric factorization. Preferred for systems under ~10,000 DOFs.                                              |
| `IterativeSolver`          | Implements `ILinearSolver` using the conjugate gradient method. Selected for large/sparse systems.                                                            |
| `SolverFactory`            | Creates the appropriate `ILinearSolver` based on `AnalysisOptions::useDirectSolver`.                                                                          |

---

## `AnalysisOptions`

| Field                  | Default | Description                                             |
| ---------------------- | ------- | ------------------------------------------------------- |
| `useDirectSolver`      | `true`  | Selects `DirectSolver`; false selects `IterativeSolver` |
| `convergenceTolerance` | `1e-9`  | Convergence threshold for iterative solver              |
| `maxIterations`        | `1000`  | Iteration cap for iterative solver                      |
| `computeReactions`     | `true`  | Compute and store support reaction forces               |
| `checkStability`       | `true`  | Check for near-singular stiffness matrix                |
| `verbose`              | `false` | Emit diagnostic messages during analysis                |

---

## `AnalysisResults`

`AnalysisResults` is the output of `AnalysisOrchestrator::analyze()`. It implements `IAnalysisResultsView` so the Infrastructure layer can consume results without coupling to this concrete type.

| Field               | Type                   | Description                               |
| ------------------- | ---------------------- | ----------------------------------------- |
| `displacements`     | `vector<Real>`         | Global DOF displacement vector (m)        |
| `reactions`         | `vector<Real>`         | Support reaction forces (N)               |
| `memberForces`      | `vector<Real>`         | Axial force per member (N, +ve = tension) |
| `memberStresses`    | `vector<Real>`         | Axial stress per member (Pa)              |
| `utilizationRatios` | `vector<Real>`         | σ / f_y per member (dimensionless)        |
| `stiffnessMatrix`   | `vector<vector<Real>>` | Full assembled global stiffness matrix    |
| `converged`         | `bool`                 | Whether the solver converged              |
| `iterations`        | `int`                  | Solver iteration count                    |
| `residualNorm`      | `Real`                 | Final residual norm                       |
| `conditionNumber`   | `Real`                 | Stiffness matrix condition number         |
| `totalDofs`         | `size_t`               | Total DOFs in system                      |
| `freeDofs`          | `size_t`               | Unconstrained DOFs                        |
| `constrainedDofs`   | `size_t`               | Constrained DOFs                          |
| `totalStrain`       | `Real`                 | Total strain energy (J)                   |
| `maxDisplacement`   | `Real`                 | Maximum nodal displacement magnitude (m)  |
| `maxStress`         | `Real`                 | Maximum axial stress in any member (Pa)   |

---

## Analysis Workflow

The `AnalysisOrchestrator::analyze(const Truss& truss, AnalysisOptions options)` method executes the following steps in sequence:

```
1. Truss::assignDofNumbers()
       ↓
2. StiffnessAssembler::assemble(truss)
       → K (totalDofs × totalDofs)
       ↓
3. Build load vector F (totalDofs × 1)
       ↓
4. BoundaryConditionHandler::getFreeDofs(truss)
       → free_dofs[]
   BoundaryConditionHandler::extractReducedSystem(K, F, free_dofs)
       → K_ff, F_f
       ↓
5. SolverFactory::create(options)
       → ILinearSolver
   ILinearSolver::solve(K_ff, F_f)
       → u_f (displacements for free DOFs)
       ↓
6. BoundaryConditionHandler::expandSolution(u_f, free_dofs, totalDofs)
       → u (full displacement vector)
       ↓
7. Post-processing:
       computeReactions(K, u, F)
       computeMemberForces(truss, u)
       computeMemberStresses(truss, results)
       computeUtilizationRatios(truss, results)
       ↓
8. Return AnalysisResults
```

---

## Solver Strategy

| Solver            | Algorithm          | Suitable For                              | Eigen Class         |
| ----------------- | ------------------ | ----------------------------------------- | ------------------- |
| `DirectSolver`    | LDLT factorization | Dense systems, small-to-medium DOF counts | `LDLT<MatrixXd>`    |
| `IterativeSolver` | Conjugate gradient | Large sparse systems (> ~10,000 DOFs)     | `ConjugateGradient` |

Selection is controlled by `AnalysisOptions::useDirectSolver`. The factory returns a `unique_ptr<ILinearSolver>`.

Both solvers throw `std::runtime_error` for singular or incompatible systems.

---

## Interaction with Other Layers

| Layer          | Interaction                                                                                                               |
| -------------- | ------------------------------------------------------------------------------------------------------------------------- |
| Core Model     | `AnalysisOrchestrator` reads `const Truss&`; results written back into node/member objects during post-processing         |
| Validation     | `TrussValidator::validate()` must pass before `AnalysisOrchestrator::analyze()` is called (enforced by Application layer) |
| Application    | `AnalysisApplicationService` invokes `AnalysisOrchestrator::analyze()` and caches `AnalysisResults` by `ResultsHandle`    |
| Infrastructure | Exporters receive `IAnalysisResultsView&` — never the concrete `AnalysisResults` struct                                   |

---

## Key Design Constraints

- `AnalysisOrchestrator` is stateless; it takes a `const Truss&` and returns `AnalysisResults` by value.
- Eigen matrices and vectors are internal to the Core and Analysis layers. They never appear in application or interface-facing headers.
- `AnalysisResults` implements `IAnalysisResultsView` via virtual methods on the struct itself — no separate adapter wrapper is needed.
- Solvers throw on failure rather than returning error codes; the Application layer wraps the call in a `try/catch` and maps exceptions to `Result<T>::Failure`.
- DOF assignment (`Truss::assignDofNumbers()`) must complete before any assembly step. The orchestrator calls it unconditionally at the start of each analysis.
