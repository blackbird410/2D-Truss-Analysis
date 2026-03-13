# GUI Architecture

## Overview

The GUI layer (`TrussAnalysisGUI`) is a Qt6 Widgets application implementing a Model–View–Controller pattern. All domain interaction occurs exclusively through `ITrussAnalysisFacade`. No widget or controller class holds a reference to any domain or application object.

The GUI is compiled under the `BUILD_GUI` CMake flag and built as the `TrussAnalysisGUI` executable (`truss-analysis`).

---

## Structural Diagram

```
┌──────────────────────────────────────────────────────────┐
│                    PRESENTATION LAYER                    │
│  MainWindow   TrussCanvasWidget   InspectorPanel         │
│  AnalysisControlBar   ResultsDockPanel   NotificationRail│
│  AnalysisOptionsDialog   StatusBar                       │
└────────────────────────┬─────────────────────────────────┘
                         │ signals/slots (handles + primitives)
┌────────────────────────▼─────────────────────────────────┐
│                    CONTROLLER LAYER                      │
│  MainWindowController  (owns all sub-controllers)        │
│  CanvasController      InspectorController               │
│  AnalysisController    ProjectController                 │
│  ExportController                                        │
└────────────────────────┬─────────────────────────────────┘
                         │ reads WorkspaceState / Qt Item Models
┌────────────────────────▼─────────────────────────────────┐
│                   SERVICE GATEWAY                        │
│  WorkspaceState (owned by MainWindowController)          │
│  NodeTableModel      MemberTableModel                    │
│  ResultsTableModel   ValidationListModel                 │
└────────────────────────┬─────────────────────────────────┘
                         │ ITrussAnalysisFacade
              ┌──────────▼──────────┐
              │  Interface Layer    │
              │ TrussAnalysisFacade │
              └─────────────────────┘
```

---

## Controllers

`MainWindowController` owns the application lifecycle and coordinates all sub-controllers. It holds the sole `ITrussAnalysisFacade*` and distributes focused sub-interfaces to each sub-controller via constructor injection.

| Controller             | Responsibility                                                                   |
| ---------------------- | -------------------------------------------------------------------------------- |
| `MainWindowController` | Owns `WorkspaceState`; coordinates state transitions; owns all sub-controllers   |
| `CanvasController`     | Translates canvas gestures (node drop, member draw, selection) into facade calls |
| `InspectorController`  | Handles property edits for selected node or member                               |
| `AnalysisController`   | Launches analysis on a background `QThread`; reports progress and results        |
| `ProjectController`    | Manages file open/save dialogs and calls facade load/save operations             |
| `ExportController`     | Manages export format dialogs and calls `exportResults` on the facade            |

---

## Qt Item Models

Qt Item Models bridge the facade's read-only view types to `QTableView` widgets. They implement `QAbstractTableModel` and refresh on `WorkspaceState` changes.

| Model                 | Data source                          | Consumer                     |
| --------------------- | ------------------------------------ | ---------------------------- |
| `NodeTableModel`      | `ITrussView::nodes()`                | Inspector node table         |
| `MemberTableModel`    | `ITrussView::members()`              | Inspector member table       |
| `ResultsTableModel`   | `IAnalysisResultsView` result arrays | ResultsDockPanel tabs        |
| `ValidationListModel` | `ValidationResult::issues()`         | Inspector validation summary |

---

## WorkspaceState

`WorkspaceState` is a value object owned by `MainWindowController`. It is the single source of truth for the UI state. All panels observe it via the `stateChanged(WorkspaceState)` signal; they never inspect each other's state directly.

```cpp
enum class WorkspacePhase { Empty, ModelBuilding, Validating, Analysing, ResultsReady };

struct WorkspaceState {
    WorkspacePhase   phase          = WorkspacePhase::Empty;
    TrussHandle      trussHandle    = 0;
    ResultsHandle    resultsHandle  = 0;
    bool             isDirty        = false;
    QString          projectName;
    QString          lastError;
};
```

Any model edit resets the phase to `ModelBuilding`, invalidating cached results.

---

## Panels and Widgets

| Component               | Type          | Responsibility                                                        |
| ----------------------- | ------------- | --------------------------------------------------------------------- |
| `TrussCanvasWidget`     | `QWidget`     | 2D structural model rendering via `QPainter`; zoom/pan; overlay modes |
| `InspectorPanel`        | `QWidget`     | Contextual property editor for selected node or member                |
| `AnalysisControlBar`    | `QWidget`     | Run/stop analysis; solver options trigger                             |
| `ResultsDockPanel`      | `QDockWidget` | Tabbed tables: Node Results, Member Results, System Summary           |
| `NotificationRail`      | `QWidget`     | Non-blocking inline alerts (success, warning, error)                  |
| `AnalysisOptionsDialog` | `QDialog`     | Solver selection and analysis parameters                              |

### Canvas Overlay Modes

| Mode           | Trigger        | Visual                                                    |
| -------------- | -------------- | --------------------------------------------------------- |
| Geometry       | Always         | Nodes, members, support symbols, force arrows             |
| Stress Ratio   | After analysis | Members colour-mapped green (0%) → red (100% utilisation) |
| Deformed Shape | After analysis | Exaggerated deformation overlaid on original geometry     |

---

## Signal/Slot Conventions

- Signals cross **one layer boundary only**: widget → controller, or controller → model/state.
- Signals carry only handles (`TrussHandle`, `ResultsHandle`) or primitive descriptors (`NodeId`, `MemberId`). Domain objects are never passed through signals.
- Cross-thread signals (from `AnalysisWorker`) use `Qt::QueuedConnection` exclusively.

### Key Signal Flows

| Emitter                | Signal                                  | Receiver               | Slot                       |
| ---------------------- | --------------------------------------- | ---------------------- | -------------------------- |
| `TrussCanvasWidget`    | `nodeDropRequested(pos, support)`       | `CanvasController`     | `onNodeDropRequested`      |
| `TrussCanvasWidget`    | `memberDrawRequested(n1, n2)`           | `CanvasController`     | `onMemberDrawRequested`    |
| `TrussCanvasWidget`    | `selectionChanged(id)`                  | `InspectorController`  | `onSelectionChanged`       |
| `InspectorPanel`       | `supportChangeRequested(nodeId, type)`  | `InspectorController`  | `onSupportChangeRequested` |
| `InspectorPanel`       | `memberPropertyChanged(memberId, spec)` | `InspectorController`  | `onMemberPropertyChanged`  |
| `AnalysisControlBar`   | `analyzeRequested(options)`             | `AnalysisController`   | `onAnalyzeRequested`       |
| `AnalysisController`   | `analysisCompleted(ResultsHandle)`      | `MainWindowController` | `onAnalysisCompleted`      |
| `MainWindowController` | `stateChanged(WorkspaceState)`          | All panels             | `onStateChanged`           |

---

## Threading

Analysis is the only operation that runs off the main thread. `AnalysisController` starts a `QThread`-based `AnalysisWorker`. The worker emits `resultReady(ResultsHandle)` via `Qt::QueuedConnection`. The main thread never blocks during analysis; the `TrussCanvasWidget` and all panels remain interactive.

---

## Theming

Two QSS themes are compiled into the application as Qt resources:

| Theme | Resource             | Background | Accent    |
| ----- | -------------------- | ---------- | --------- |
| Dark  | `:/themes/dark.qss`  | `#1E2028`  | `#5B9BD5` |
| Light | `:/themes/light.qss` | `#F5F5F5`  | `#1565C0` |

The active theme is persisted via `QSettings`. No theme-selection logic exists in widget code — widgets use `setObjectName` and the stylesheet handles styling.

---

## Toolkit Decision: Qt Widgets

Qt Widgets was selected over Qt Quick (QML) for the following reasons:

- The primary rendering element is a custom 2D canvas requiring `QPainter` with coordinate transforms, zoom, and pan — natively supported by `QWidget::paintEvent`.
- Tabular result panels map directly to `QTableView` + `QAbstractTableModel`.
- Dockable panels are a single `QDockWidget` call.
- Qt Widgets has strong precedent in engineering applications (Qt Creator, KiCad, FreeCAD).
- Qt Quick offers no advantage that outweighs its added complexity for this use case.
