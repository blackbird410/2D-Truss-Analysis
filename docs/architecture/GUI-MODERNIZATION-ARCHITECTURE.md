# GUI Modernization Architecture

## 2D Truss Analysis — New UI Design

**Author:** Principal Software Architect / Senior Qt UI Engineer  
**Date:** March 2, 2026  
**Version Target:** 3.0.0  
**Status:** Design Proposal

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Project Scope & Capability Analysis](#2-project-scope--capability-analysis)
3. [Research Findings](#3-research-findings)
4. [Architectural Recommendation](#4-architectural-recommendation)
5. [UI Structural Design](#5-ui-structural-design)
6. [Component Specifications](#6-component-specifications)
7. [Integration Strategy](#7-integration-strategy)
8. [Validation Checklist](#8-validation-checklist)

---

## 1. Executive Summary

The new GUI for the 2D Truss Analysis application is designed as a **professional-grade structural analysis workbench**. It replaces any previously existing UI with a ground-up design anchored to the `ITrussAnalysisFacade` interface. No domain knowledge is embedded in any view class. All business interaction travels exclusively through the facade boundary.

The design adopts a **workspace-centric layout** (analogous to professional CAD/FEM tools) divided into a canvas area (primary), an inspector panel (right), a history/log panel (bottom), and a toolbar strip. A Model–View–Controller architecture mediates all state transitions. Qt Widgets is selected as the rendering toolkit for reasons justified in Section 4.

Key quality attributes achieved:

| Attribute      | Mechanism                                                               |
| -------------- | ----------------------------------------------------------------------- |
| Correctness    | All mutations go through `ITrussService`; no direct domain access       |
| Responsiveness | Analysis runs on a `QThread`; UI never blocks                           |
| Testability    | Every controller depends on an interface, not a concrete class          |
| Extensibility  | New panels, tools, and themes are added without modifying existing ones |
| Accessibility  | WCAG-aligned color contrast; keyboard-navigable toolbar                 |

---

## 2. Project Scope & Capability Analysis

### 2.1 Core Domain Capabilities (from `ITrussAnalysisFacade`)

The facade exposes the following capability groups:

| Group                   | Key Operations                                                                                           |
| ----------------------- | -------------------------------------------------------------------------------------------------------- |
| **Truss Lifecycle**     | `createTruss`, `loadTruss`, `saveTruss`, `clearTruss`, `clearAll`                                        |
| **Truss Editing**       | `addNode`, `addMember`, `removeNode`, `removeMember`, `setNodeSupport`, `applyNodeLoad`, `clearNodeLoad` |
| **Validation**          | `validateTruss`, `validateFromFile`                                                                      |
| **Analysis Execution**  | `analyze(truss, options)`, `analyzeFromFile`, `analyzeInteractive`                                       |
| **Results Access**      | `getResultsView(handle)`, `getTrussView(handle)`                                                         |
| **Export**              | `exportResults(handle, format, filepath)` — CSV, JSON, HTML, LaTeX, Text, XML                            |
| **Resource Management** | Handle invalidation: `clearTruss(handle)`, `clearResults(handle)`, `isValidTrussHandle`                  |

#### 2.1.1 Data Contracts

```
ITrussView → NodeView { id, x, y, support, fx, fy, dx, dy, rx, ry }
           → MemberView { id, startNode, endNode, E, A, length, angle,
                          axialForce, axialStress, utilizationRatio, yielded }

IAnalysisResultsView → displacements[], reactions[], memberForces[],
                        memberStresses[], utilizationRatios[]
                      → converged, iterations, residualNorm, conditionNumber
                      → totalDofs, freeDofs, constrainedDofs
                      → totalStrain, maxDisplacement, maxStress
```

#### 2.1.2 Command / Query Boundary

| Category | Operations                                                                                                                  |
| -------- | --------------------------------------------------------------------------------------------------------------------------- |
| Commands | `addNode`, `addMember`, `removeNode`, `removeMember`, `setNodeSupport`, `applyNodeLoad`, `analyze`, `save`, `load`, `clear` |
| Queries  | `getTrussView`, `getResultsView`, `validateTruss`, `isValidTrussHandle`, `isValidResultsHandle`                             |

### 2.2 Primary User Workflows

| Workflow                | Steps                                                           | Frequency |
| ----------------------- | --------------------------------------------------------------- | --------- |
| Build & Analyze         | Create → Add Nodes → Add Members → Apply Loads → Analyze → View | Very High |
| Load from File & Verify | Open File → Review Model → Validate → Analyze                   | High      |
| Iterate on Design       | Analyze → Inspect Results → Edit Model → Re-analyze             | High      |
| Export Results          | Analyze → Export (CSV/JSON/HTML/LaTeX)                          | Medium    |
| Validate Only           | Load/Build → Validate → Review Issues                           | Medium    |
| Save Project            | Build/Edit → Save to JSON/XML                                   | Medium    |

### 2.3 User Personas

**Persona A — Civil Engineering Student**

- Builds simple trusses (3–15 members), learns structural behaviour
- Needs clear feedback, visual guides, descriptive error messages
- Primarily uses the canvas; inspects tabular results after analysis

**Persona B — Structural Analysis Practitioner / Reviewer**

- Loads complex models from JSON/XML, verifies results against hand calculations
- Needs precise numeric output, stiffness matrix inspection, export to LaTeX/CSV
- Values precision, efficiency, keyboard shortcuts

**Persona C — Software Developer / QA Engineer**

- Runs the GUI to verify functional correctness during development
- Needs diagnostic output (condition number, DOF count, convergence status)

### 2.4 UI Responsibility Breakdown

| UI Component               | Responsibility                                                         | Persona |
| -------------------------- | ---------------------------------------------------------------------- | ------- |
| Canvas (drawing area)      | Visualise truss geometry; accept pointer gestures for node/member edit | A, B    |
| Inspector Panel            | Contextual editor for selected node/member properties                  | A, B    |
| Analysis Control Bar       | Launch analysis, show solver options, trigger export                   | A, B    |
| Results Panel              | Tabular and colour-mapped results; per-member force/stress/ratio       | A, B, C |
| Status Bar                 | Current validation state, analysis convergence summary                 | A, B, C |
| Notification System        | Non-blocking inline alerts for errors, warnings, success               | A, B    |
| File Operations Area       | Open, Save, Export dialogs                                             | A, B    |
| Material / Section Library | Quick-pick material and section specs for member creation              | A       |

### 2.5 Facade-Driven Interaction Matrix

| UI Action                 | Facade Call                                   | Expected Response                          |
| ------------------------- | --------------------------------------------- | ------------------------------------------ |
| Drop node on canvas       | `addNode(h, pos, support)`                    | `Result<NodeId>` → redraw                  |
| Draw member between nodes | `addMember(h, n1, n2, mat, sec)`              | `Result<MemberId>` → redraw                |
| Change support type       | `setNodeSupport(h, nodeId, type)`             | `Result<bool>` → redraw                    |
| Apply load arrow          | `applyNodeLoad(h, nodeId, force)`             | `Result<bool>` → redraw                    |
| Delete selected node      | `removeNode(h, nodeId)`                       | `Result<bool>` → redraw                    |
| Open JSON/XML file        | `loadTruss(filepath)`                         | `Result<TrussHandle>` → redraw             |
| Save project              | `saveTruss(h, filepath)`                      | `Result<bool>` → confirm                   |
| Validate                  | `validateTruss(h)`                            | `ValidationResult` → badge                 |
| Run analysis              | `analyze(truss, opts)` (on background thread) | `Result<ResultsHandle>` → populate results |
| Export results            | `exportResults(rh, format, path)`             | `bool` → confirm dialog                    |
| Clear all                 | `clearAll()`                                  | Reset all UI state                         |

---

## 3. Research Findings

### 3.1 Modern Desktop Application Design Principles

#### 3.1.1 Minimalism and Clarity

Engineering tools benefit from deliberately minimal chrome. Each pixel not occupied by structural content competes with meaningful data. The new UI adopts a **two-column + canvas** split: the canvas occupies ~65% of horizontal space; controls and results share the remaining 35%. No floating dialogs exist during active editing — all inputs occur in-place within docked panels.

#### 3.1.2 Progressive Disclosure

Not all controls are visible simultaneously. The **Inspector Panel** is context-sensitive:

- No selection → shows project metadata and validator summary
- Node selected → shows position, support type, applied load fields
- Member selected → shows material picker, section fields, analysis results for that member

This prevents the cognitive overload that arises when all parameters are presented at once.

#### 3.1.3 Visual Hierarchy

A three-tier hierarchy is enforced:

| Tier | Content                        | Visual Weight      |
| ---- | ------------------------------ | ------------------ |
| 1    | Canvas (structural model)      | Dominant / central |
| 2    | Inspector & Results Panels     | Supporting         |
| 3    | Status Bar / Notification Rail | Peripheral         |

Typography and spacing are governed by a 4px base grid (element padding = 4, 8, 12, 16, 24px steps).

#### 3.1.4 Feedback Systems

Every user action should produce a response within 100 ms (visual) or an asynchronous indicator (spinner) within 200 ms (analysis). Specific mechanisms:

- **Inline validation badges** on the canvas (node count, member count, DOF count) update on every edit
- **Progress indicator** replaces the "Run Analysis" button while computation is in flight
- **Non-intrusive toast notifications** appear in the status rail for non-error conditions
- **Error panels** appear inline in the Inspector for validation failures affecting the selection

#### 3.1.5 Error Prevention and Recovery

- Member creation between non-existent nodes is blocked at the widget level before calling the facade
- Attempting analysis on a structurally incomplete truss shows the validation report without dismissal
- Destructive actions (clear all, delete) require a single confirmation step (no modal dialogs — use an inline undo banner instead where possible)

#### 3.1.6 Discoverability

- The main toolbar contains only the seven most frequent operations with labelled icons
- Rarely used operations (stiffness matrix export, solver selection) are in the Settings panel and Analysis Options dialog
- Contextual right-click menus on canvas elements expose element-specific actions

### 3.2 Engineering-Oriented Application Interfaces

From study of professional structural analysis tools (e.g., SAP2000, RFEM, ETABS student editions) and analysis IDE patterns:

**Finding 1 — Visualization-First Layout**: All professional-grade analysis tools foreground the model visualisation. Input forms are secondary panels, never primary views.

**Finding 2 — Dual-View Results**: A heat-map / colour-coded overlay on the structural diagram (member stress ratio) paired with a tabular breakdown provides faster comprehension than a standalone table.

**Finding 3 — Immediate Validation Feedback**: Engineers re-run analysis on every minor change. The UI should display validator state (✓ / ⚠ / ✗) at all times, not only after an explicit "validate" action.

**Finding 4 — Sticky Inspector State**: When switching between node and member data, the inspector height should not collapse; it should transition its content smoothly.

**Finding 5 — Export is a Terminal Action**: Export dialogs appear at the end of a workflow and benefit from a preview of what will be written (format, file size estimate, content sections).

### 3.3 Qt-Specific Best Practices

#### 3.3.1 Qt Widgets vs Qt Quick (QML)

| Criterion                         | Qt Widgets                          | Qt Quick (QML)                       |
| --------------------------------- | ----------------------------------- | ------------------------------------ |
| Custom 2D rendering (QPainter)    | **Native, first-class**             | Requires Canvas element or C++ scene |
| Accessibility (QAccessible)       | **Mature, automatic**               | Requires manual bindings             |
| Tabular data (QAbstractItemModel) | **Mature, QTableView out-of-box**   | Requires TableView reinterpretation  |
| Dockable panels (QDockWidget)     | **Native**                          | Not available                        |
| Styling (QSS)                     | Flexible, CSS-like                  | QML property bindings                |
| Engineering app precedent         | Strong (Qt Creator, KiCad, FreeCAD) | Primarily consumer/embedded products |
| Thread-safe UI updates            | `QMetaObject::invokeMethod` pattern | Same                                 |
| Build complexity                  | Lower                               | Higher (qmldir, Qt Quick modules)    |

**Decision: Qt Widgets.** The application's core visual element is a custom 2D canvas requiring `QPainter` with coordinate transforms, zoom, and pan. Qt Widgets provides this natively through `QWidget::paintEvent`. The tabular results panels map directly to `QTableView` + `QAbstractItemModel`. Dockable panels are a single `QDockWidget` call. Qt Quick offers no advantage that outweighs its added complexity for this use case.

#### 3.3.2 MVC Architecture Pattern in Qt

Each data-bearing panel follows the Qt Model–View pattern:

- **Model**: thin wrapper that queries `ITrussView` / `IAnalysisResultsView` and implements `QAbstractTableModel`
- **View**: `QTableView` with a custom `QStyledItemDelegate` for colour mapping
- **Controller**: `QObject`-derived mediator that calls the facade on user actions and notifies the model to refresh

#### 3.3.3 Signal/Slot Usage

- Signals cross **one layer boundary only**: View → Controller, Controller → Model/View
- Signals never carry domain objects — they carry handles (`TrussHandle`, `ResultsHandle`) or primitive descriptors (`NodeId`, `MemberId`)
- Cross-thread signals use `Qt::QueuedConnection` exclusively

#### 3.3.4 State Management

All UI state is expressed as a `WorkspaceState` value object owned by `MainWindowController`. Views observe it via signals; they never inspect each other's state directly.

```
enum class WorkspacePhase { Empty, ModelBuilding, Validating, Analysing, ResultsReady };

struct WorkspaceState {
    WorkspacePhase           phase          = WorkspacePhase::Empty;
    application::TrussHandle trussHandle    = 0;
    application::ResultsHandle resultsHandle= 0;
    bool                     isDirty        = false;
    QString                  projectName;
    QString                  lastError;
};
```

#### 3.3.5 Responsive Layout Strategy

- Minimum window size: 1024 × 768 px
- Target design size: 1440 × 900 px
- `QSplitter` separates canvas from inspector; user-adjustable, with a remembered default ratio of 65/35
- The bottom log/results panel is a `QDockWidget` (dockable, hideable, floatable)
- All font sizes reference `QApplication::font().pointSize()` as a base — no hardcoded pt values

#### 3.3.6 Styling Architecture (QSS)

Two themes are provided as `.qss` files loaded at startup:

- `:/themes/dark.qss` — graphite background (#1E2028), steel-blue accent (#5B9BD5), amber warning (#FFC107)
- `:/themes/light.qss` — near-white background (#F5F5F5), navy accent (#1565C0), orange warning (#E65100)

The active theme is persisted to `QSettings`. No theme-selection logic is embedded in widget code — widgets use `setObjectName` and the stylesheet does all the rest.

---

## 4. Architectural Recommendation

### 4.1 Chosen Architecture: Clean Qt MVC with a Service Gateway

```
┌───────────────────────────────────────────────────────────────────┐
│                        GUI MODULE                                 │
│                                                                   │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │                  PRESENTATION LAYER                         │  │
│  │  MainWindow  CanvasPanel  InspectorPanel  ResultsPanel      │  │
│  │  ToolBar     StatusBar   NotificationRail                   │  │
│  └──────────────────────┬──────────────────────────────────────┘  │
│                         │ signals/slots (handles + primitives)    │
│  ┌──────────────────────▼──────────────────────────────────────┐  │
│  │                   CONTROLLER LAYER                          │  │
│  │  MainWindowController   CanvasController                    │  │
│  │  InspectorController    AnalysisController                  │  │
│  │  ProjectController      ExportController                    │  │
│  └──────────────────────┬──────────────────────────────────────┘  │
│                         │ calls                                   │
│  ┌──────────────────────▼──────────────────────────────────────┐  │
│  │               SERVICE GATEWAY (read model)                  │  │
│  │  WorkspaceState (owned by MainWindowController)             │  │
│  │  Qt Item Models: NodeTableModel, MemberTableModel,          │  │
│  │                  ResultsTableModel, ValidationListModel     │  │
│  └──────────────────────┬──────────────────────────────────────┘  │
│                         │                                         │
└─────────────────────────┼─────────────────────────────────────────┘
                          │ ITrussAnalysisFacade (interface boundary)
              ┌───────────▼───────────┐
              │   Interface Layer     │
              │  TrussAnalysisFacade  │
              └───────────────────────┘
```

### 4.2 Key Decisions

| Decision                                   | Justification                                                                                                                                        |
| ------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| Single facade injection point              | Only `MainWindowController` holds the `ITrussAnalysisFacade*`; all controllers receive pinpoint sub-interfaces through constructor injection from it |
| Qt Item Models as read model               | Bridges domain view types to Qt's MVC system without coupling                                                                                        |
| `WorkspaceState` as single source of truth | Eliminates inter-panel coupling; panels react to state transitions                                                                                   |
| Background thread for analysis only        | Analysis is the only IO/compute-heavy operation; everything else is synchronous                                                                      |
| Theme as pure QSS resources                | Zero widget-code involvement in theme switching                                                                                                      |

---

## 5. UI Structural Design

### 5.1 Main Window Layout

```
┌───────────────────────────────────────────────────────────────────────────────────────┐
│  Menu Bar: File | Edit | View | Analysis | Export | Help                              │
├───────────────────────────────────────────────────────────────────────────────────────┤
│  Tool Strip:  [New][Open][Save] │ [Select][Node][Member][Load][Support] │ [Run][Stop] │
├────────────────────────────────────────────────┬──────────────────────────────────────┤
│                                                │                                      │
│                                                │   INSPECTOR PANEL                    │
│         CANVAS PANEL                           │  ┌───────────────────────┐           │
│         (TrussCanvasWidget)                    │  │ [Project / Selection] │           │
│                                                │  └───────────────────────┘           │
│   ┌─────────────────────────────────────┐      │  ┌───────────────────────┐           │
│   │  Structural model drawn here        │      │  │ Properties editor     │           │
│   │  Pan: middle-click drag             │      │  │ (context-sensitive)   │           │
│   │  Zoom: scroll wheel                 │      │  └───────────────────────┘           │
│   │  Overlay: force arrows, reactions,  │      │  ┌───────────────────────┐           │
│   │           deformed shape, colours   │      │  │ Validation Summary    │           │
│   └─────────────────────────────────────┘      │  └───────────────────────┘           │
│                                                │  ┌───────────────────────┐           │
│  Minimap (bottom-right corner)                 │  │ Analysis Options      │           │
│                                                │  └───────────────────────┘           │
├────────────────────────────────────────────────┴──────────────────────────────────────┤
│  RESULTS / LOG DOCK (QDockWidget — bottom, dismissible)                               │
│  Tabs: [Node Results] [Member Results] [System Summary] [Stiffness Matrix]            │
├───────────────────────────────────────────────────────────────────────────────────────┤
│  Status Bar: [Phase badge] [Node: N  Member: M  DOF: D] [Last operation…]             │
└───────────────────────────────────────────────────────────────────────────────────────┘
```

### 5.2 Canvas Overlay Modes

The canvas supports three mutually exclusive display modes toggled from the toolbar:

| Mode           | Visual                                                                      | Active when      |
| -------------- | --------------------------------------------------------------------------- | ---------------- |
| Geometry       | Plain nodes (circles), members (lines), support symbols, force arrows       | Always available |
| Stress Ratio   | Members colour-mapped from green (0%) to red (100% utilisation) with legend | After analysis   |
| Deformed Shape | Exaggerated deformed geometry overlaid on original in ghost colour          | After analysis   |

### 5.3 Navigation Model

**Decision: Workspace-Driven Layout (not wizard-based)**

Justification:

- Engineers do not follow a linear workflow; they iterate freely between editing and analysis
- A wizard enforces sequential steps and blocks non-linear interaction
- A persistent workspace (canvas + inspector always visible) matches the mental model of structural analysis software
- Wizard-based navigation is appropriate only for first-run setup or complex multi-step forms; neither applies here

The workspace progresses through `WorkspacePhase` states that adjust which controls are enabled, not which screen is shown:

```
Empty → ModelBuilding → Validating → Analysing → ResultsReady
          ↑_____________↑_____________↑____________↑
              (any editing action resets to ModelBuilding)
```

### 5.4 Visual Design System

#### 5.4.1 Colour System

| Token                  | Dark Theme | Light Theme | Purpose                          |
| ---------------------- | ---------- | ----------- | -------------------------------- |
| `--bg-primary`         | `#1E2028`  | `#F5F5F5`   | Window and panel background      |
| `--bg-secondary`       | `#252830`  | `#FFFFFF`   | Card / groupbox background       |
| `--bg-canvas`          | `#14161C`  | `#FAFAFA`   | Canvas drawing area              |
| `--accent-primary`     | `#5B9BD5`  | `#1565C0`   | Buttons, selection highlights    |
| `--accent-hover`       | `#7BB3E0`  | `#1976D2`   | Hover states                     |
| `--text-primary`       | `#E8EAED`  | `#1A1A1A`   | Primary readable text            |
| `--text-secondary`     | `#9AA0A6`  | `#5F5F5F`   | Labels, placeholders             |
| `--status-ok`          | `#34A853`  | `#2E7D32`   | Validation pass, convergence     |
| `--status-warning`     | `#FFC107`  | `#E65100`   | Validation warnings              |
| `--status-error`       | `#EA4335`  | `#C62828`   | Validation errors, failed states |
| `--member-tension`     | `#4FC3F7`  | `#0288D1`   | Members in tension               |
| `--member-compression` | `#FF7043`  | `#D84315`   | Members in compression           |
| `--member-yield`       | `#FF1744`  | `#B71C1C`   | Members at/above yield           |

#### 5.4.2 Typography Scale (relative to `QApplication::font().pointSize()`)

| Level | Multiplier | Usage                                  |
| ----- | ---------- | -------------------------------------- |
| H1    | ×1.5       | Panel section titles (rare)            |
| H2    | ×1.25      | Group box labels                       |
| Body  | ×1.0       | Inputs, table cells, general text      |
| Small | ×0.85      | Status bar, tooltips, secondary labels |
| Code  | ×0.9       | Numeric output, coordinate display     |

Numeric output uses a monospace fallback (`"Consolas", "Liberation Mono", monospace`).

#### 5.4.3 Spacing System (4px base grid)

| Token | Value | Usage                 |
| ----- | ----- | --------------------- |
| `xs`  | 4px   | Icon-to-label spacing |
| `sm`  | 8px   | Intra-group padding   |
| `md`  | 12px  | Panel content margin  |
| `lg`  | 16px  | Section separator     |
| `xl`  | 24px  | Panel-to-panel gap    |

### 5.5 Interaction Flow Diagram

```
[User drops node on canvas]
       │
       ▼
CanvasWidget::mousePressEvent()
       │ emits nodeDropRequested(pos, supportType)
       ▼
CanvasController::onNodeDropRequested(pos, supportType)
       │ calls facade->addNode(trussHandle, pos, supportType)
       ▼
 Result<NodeId>
  ├── success → emit trussModified(trussHandle)
  │      │
  │      ▼
  │  MainWindowController::onTrussModified()
  │      │ refreshes NodeTableModel, MemberTableModel
  │      │ emits stateChanged(WorkspaceState{phase=ModelBuilding, dirty=true})
  │      ▼
  │  CanvasWidget::refresh()  + InspectorPanel::refresh()  + StatusBar::refresh()
  │
  └── failure → emit operationFailed(errorMsg)
         │
         ▼
     NotificationRail::showError(msg)
```

### 5.6 Signal–Slot Interaction Map

| Emitter                   | Signal                                  | Receiver               | Slot                       |
| ------------------------- | --------------------------------------- | ---------------------- | -------------------------- |
| `CanvasWidget`            | `nodeDropRequested(pos, support)`       | `CanvasController`     | `onNodeDropRequested`      |
| `CanvasWidget`            | `memberDrawRequested(n1, n2)`           | `CanvasController`     | `onMemberDrawRequested`    |
| `CanvasWidget`            | `selectionChanged(NodeId/MemberId)`     | `InspectorController`  | `onSelectionChanged`       |
| `InspectorPanel`          | `supportChangeRequested(nodeId, type)`  | `InspectorController`  | `onSupportChangeRequested` |
| `InspectorPanel`          | `loadChangeRequested(nodeId, force)`    | `InspectorController`  | `onLoadChangeRequested`    |
| `InspectorPanel`          | `memberPropertyChanged(memberId, spec)` | `InspectorController`  | `onMemberPropertyChanged`  |
| `AnalysisControlBar`      | `analyzeRequested(options)`             | `AnalysisController`   | `onAnalyzeRequested`       |
| `AnalysisController`      | `analysisStarted()`                     | `MainWindowController` | `onAnalysisStarted`        |
| `AnalysisController`      | `analysisCompleted(ResultsHandle)`      | `MainWindowController` | `onAnalysisCompleted`      |
| `AnalysisController`      | `analysisFailed(errorMsg)`              | `NotificationRail`     | `showError`                |
| `MainWindowController`    | `stateChanged(WorkspaceState)`          | All panels             | `onStateChanged`           |
| `ProjectController`       | `trussLoaded(TrussHandle, name)`        | `MainWindowController` | `onTrussLoaded`            |
| `ExportController`        | `exportCompleted(filepath)`             | `NotificationRail`     | `showSuccess`              |
| `AnalysisWorker` (thread) | `resultReady(ResultsHandle)` (queued)   | `AnalysisController`   | `onWorkerResultReady`      |

---

## 6. Component Specifications

### 6.1 `MainWindow`

**File:** `src/gui/main_window.hpp` / `.cpp`  
**Base class:** `QMainWindow`

| Attribute         | Specification                                                                               |
| ----------------- | ------------------------------------------------------------------------------------------- |
| Responsibility    | Top-level host: assembles and wires all panels, controllers, and models                     |
| Facade dependency | Receives `ITrussAnalysisFacade&` at construction; distributes sub-interfaces to controllers |
| Layout            | `QSplitter(CanvasInspector)`as central widget;`QDockWidget(ResultsPanel)` at bottom         |
| Lifetime          | Owns all child controllers via `std::unique_ptr`; owns all Qt Item Models                   |
| Threading         | Constructs `AnalysisWorker` and moves it to its thread in constructor                       |
| State             | Holds `WorkspaceState`; broadcasts via `stateChanged` signal                                |
| Error handling    | Catches `std::exception` at the top-level slot boundary; forwards to `NotificationRail`     |

**Constructor signature:**

```cpp
explicit MainWindow(interface::ITrussAnalysisFacade& facade,
                    QWidget* parent = nullptr);
```

No controller, presenter, or model is passed in from outside — `MainWindow` constructs them from the facade.

---

### 6.2 `TrussCanvasWidget`

**File:** `src/gui/widgets/truss_canvas_widget.hpp` / `.cpp`  
**Base class:** `QWidget`

| Attribute           | Specification                                                                                          |
| ------------------- | ------------------------------------------------------------------------------------------------------ |
| Responsibility      | Render the truss model; accept pointer/keyboard input for structural editing                           |
| Data input          | Receives a const `ITrussView*` snapshot pointer on each `refresh(state)` call                          |
| No facade access    | Emits signals only; never calls facade directly                                                        |
| Rendering           | `QPainter` with coordinate transform (`m_worldToScreen`): pan + zoom maintained as `QTransform`        |
| Overlays            | `DisplayMode` enum: `Geometry` / `StressRatio` / `DeformedShape`; toggled externally                   |
| Interaction signals | `nodeDropRequested`, `memberDrawRequested`, `loadDropRequested`, `selectionChanged`, `deleteRequested` |
| Canvas state        | `DrawingMode` (Select / AddNode / AddMember / AddLoad / SetSupport) driven by toolbar                  |
| Keyboard shortcuts  | `Esc` → Select mode; `N` → AddNode; `M` → AddMember; `Del` → delete selection                          |
| No domain types     | Uses `core::NodeId`, `core::MemberId` (primitive aliases) and `core::Point2D` only in signals          |

**Rendering pipeline (`paintEvent`):**

1. Fill background with `--bg-canvas`
2. Draw grid lines (optional, from settings)
3. Draw members (colour per `DisplayMode`)
4. Draw nodes (circle + support symbol)
5. Draw force/moment arrows
6. Draw deformed shape overlay (if `ResultsOverlay` provided)
7. Draw interaction in-progress ghost (member being drawn)
8. Draw selection highlight

---

### 6.3 `InspectorPanel`

**File:** `src/gui/panels/inspector_panel.hpp` / `.cpp`  
**Base class:** `QWidget`

| Attribute         | Specification                                                                                                                                     |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| Responsibility    | Context-sensitive property editor and summary view                                                                                                |
| Sections          | `ProjectSummarySection`, `NodeEditorSection`, `MemberEditorSection`, `ValidationSummarySection`                                                   |
| Content switching | A `QStackedWidget` with four pages; active page controlled by `InspectorController`                                                               |
| No facade access  | Emits change-request signals only                                                                                                                 |
| Node editor       | Inputs: X, Y (read-only when from file), support type `QComboBox`, Fx/Fy `QDoubleSpinBox`                                                         |
| Member editor     | Input: material `QComboBox` (from `MaterialLibraryService`), area `QDoubleSpinBox`; reads: length (display-only), angle, axial force/stress/ratio |
| Validation view   | Scrollable list of `ValidationIssue` entries; severity icon prefix; click-to-select-element                                                       |
| Error display     | Inline `QLabel` with ⚠ icon per invalid field; no modal popups                                                                                    |

---

### 6.4 `AnalysisControlBar`

**File:** `src/gui/panels/analysis_control_bar.hpp` / `.cpp`  
**Base class:** `QWidget`

| Attribute         | Specification                                                                                |
| ----------------- | -------------------------------------------------------------------------------------------- |
| Responsibility    | Initiates analysis, displays run status, exposes solver configuration                        |
| Primary action    | "Run Analysis" `QPushButton` — transitions to progress indicator during analysis             |
| Secondary actions | "Validate" button; "Analysis Options" gear icon (opens `AnalysisOptionsDialog`)              |
| State response    | Disabled during `Analysing` phase; "Stop" button appears (future: cancellation support)      |
| Solver options    | `QComboBox` for Direct/Iterative solver; tolerance `QDoubleSpinBox` (progressive disclosure) |
| Signals emitted   | `analyzeRequested(AnalysisOptions)`, `validateRequested()`                                   |
| No facade access  | —                                                                                            |

---

### 6.5 `ResultsDockPanel`

**File:** `src/gui/panels/results_dock_panel.hpp` / `.cpp`  
**Base class:** `QDockWidget`

| Attribute          | Specification                                                                                        |
| ------------------ | ---------------------------------------------------------------------------------------------------- |
| Responsibility     | Read-only display of all analysis results and system metadata                                        |
| Tabs               | `QTabWidget`: Node Results / Member Results / System Summary / Stiffness Matrix                      |
| Node Results tab   | `QTableView` bound to `NodeResultsTableModel` (id, dx, dy, rx, ry)                                   |
| Member Results tab | `QTableView` bound to `MemberResultsTableModel` (id, force, stress, ratio, state)                    |
| Member colouring   | `QStyledItemDelegate` paints ratio column with gradient fill matching canvas overlay                 |
| System Summary tab | Key-value `QFormLayout`: max displacement, max stress, converged (✓/✗), condition number, DOF counts |
| Stiffness Matrix   | `QTableWidget` (read-only) populated on demand; shown only when tab is active (lazy)                 |
| Export button      | Per-tab export to applicable formats; triggers `ExportController`                                    |
| Empty state        | Placeholder label "Run analysis to see results" when `ResultsHandle == 0`                            |

---

### 6.6 `StatusBar`

**File:** Inline in `main_window.cpp` (thin component, no separate class warranted)

| Attribute      | Specification                                                                |
| -------------- | ---------------------------------------------------------------------------- |
| Segments       | `[Phase Badge] [Model Stats] [Separator] [Last Operation] [Progress Widget]` |
| Phase Badge    | Coloured chip: `EMPTY` / `EDITING` / `VALID` / `RUNNING` / `RESULTS`         |
| Model Stats    | "Nodes: N Members: M DOFs: D Constraints: C" — updated on every edit         |
| Last Operation | Short one-line confirmation of the last completed action                     |
| Progress       | `QProgressBar` (indeterminate mode) visible only during `Analysing` phase    |

---

### 6.7 `NotificationRail`

**File:** `src/gui/widgets/notification_rail.hpp` / `.cpp`  
**Base class:** `QWidget`

| Attribute      | Specification                                                                                                                              |
| -------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Responsibility | Non-blocking, auto-dismissing inline notification system below the toolbar                                                                 |
| Types          | `Info` (blue) / `Success` (green) / `Warning` (amber) / `Error` (red)                                                                      |
| Behaviour      | Slides in from top with 150ms animation; auto-dismisses after 4s for Info/Success; persists for Error until user dismisses or next success |
| Capacity       | Stacks up to three simultaneous notifications; older ones slide out                                                                        |
| No modal       | Never uses `QMessageBox` for operational feedback — only for irreversible confirm dialogs                                                  |

---

### 6.8 Controllers

#### 6.8.1 `MainWindowController`

| Attribute     | Value                                                                                       |
| ------------- | ------------------------------------------------------------------------------------------- |
| Owns          | `WorkspaceState`; all sub-controllers; re-emits `stateChanged`                              |
| Facade access | Holds `ITrussAnalysisFacade*`; distributes to sub-controllers                               |
| Key slots     | `onTrussLoaded`, `onTrussModified`, `onAnalysisCompleted`, `onAnalysisFailed`, `onClearAll` |
| Key signals   | `stateChanged(WorkspaceState)`                                                              |

#### 6.8.2 `CanvasController`

| Attribute     | Value                                                               |
| ------------- | ------------------------------------------------------------------- |
| Facade access | `ITrussService*` (subset of facade)                                 |
| Key slots     | `onNodeDropRequested`, `onMemberDrawRequested`, `onDeleteRequested` |
| Key signals   | `trussModified(TrussHandle)`, `operationFailed(QString)`            |

#### 6.8.3 `InspectorController`

| Attribute     | Value                                                                                                                 |
| ------------- | --------------------------------------------------------------------------------------------------------------------- |
| Facade access | `ITrussService*`                                                                                                      |
| Key slots     | `onSelectionChanged(NodeId/MemberId)`, `onSupportChangeRequested`, `onLoadChangeRequested`, `onMemberPropertyChanged` |
| Key signals   | `trussModified(TrussHandle)`, `operationFailed(QString)`                                                              |

#### 6.8.4 `AnalysisController`

| Attribute     | Value                                                                                        |
| ------------- | -------------------------------------------------------------------------------------------- |
| Facade access | `IAnalysisService*`, `ITrussService*`                                                        |
| Threading     | Constructs `AnalysisWorker`; moves to `QThread`; connects via `Qt::QueuedConnection`         |
| Key slots     | `onAnalyzeRequested(options)`, `onValidateRequested()`, `onWorkerResultReady(ResultsHandle)` |
| Key signals   | `analysisStarted()`, `analysisCompleted(ResultsHandle)`, `analysisFailed(QString)`           |

#### 6.8.5 `ProjectController`

| Attribute         | Value                                                                                                      |
| ----------------- | ---------------------------------------------------------------------------------------------------------- |
| Facade access     | `ITrussService*` (load/save/create/clear)                                                                  |
| Key slots         | `onNewProjectRequested`, `onOpenFileRequested`, `onSaveRequested`, `onSaveAsRequested`, `onCloseRequested` |
| File format       | Auto-detects from extension; manual override via dialog format combo                                       |
| Dirty-state guard | Checks `WorkspaceState.isDirty` before destructive actions; shows inline confirmation banner               |

#### 6.8.6 `ExportController`

| Attribute     | Value                                                |
| ------------- | ---------------------------------------------------- |
| Facade access | `IAnalysisService*`                                  |
| Key slots     | `onExportRequested(ResultsHandle, format, filepath)` |
| Key signals   | `exportCompleted(filepath)`, `exportFailed(QString)` |

---

### 6.9 Qt Item Models

#### 6.9.1 `NodeTableModel` (`QAbstractTableModel`)

Columns: ID | X | Y | Support | Fx | Fy | dx | dy | Rx | Ry

- Populated by calling `facade.getTrussView(handle).getNodeViews()`
- Refreshed entirely on `trussModified` or `analysisCompleted` signal
- `data(Qt::DisplayRole)` formats floats to 6 significant figures using `QLocale`
- `data(Qt::BackgroundRole)` highlights nodes with applied loads

#### 6.9.2 `MemberTableModel` (`QAbstractTableModel`)

Columns: ID | Start | End | Material | E [GPa] | A [cm²] | Length | Angle | Force [kN] | Stress [MPa] | Ratio | State

- Mixed pre/post-analysis columns: force/stress/ratio show "—" before analysis
- `data(Qt::ForegroundRole)` colours the Ratio column using the stress-ratio colour scale
- `data(Qt::ToolTipRole)` shows "Yielded" badge when `yielded == true`

#### 6.9.3 `ValidationListModel` (`QAbstractListModel`)

- Each item is a `ValidationIssue` from `core::validation::ValidationResult`
- `data(Qt::DecorationRole)` returns severity icon
- `data(Qt::UserRole)` returns affected `NodeId`/`MemberId` for click-to-select

---

## 7. Integration Strategy

### 7.1 File Structure Plan

```
src/gui/
├── main.cpp                          # Entry point: constructs facade, launches MainWindow
├── main_window.hpp/.cpp              # QMainWindow host, wires all components
│
├── controllers/
│   ├── main_window_controller.hpp/.cpp
│   ├── canvas_controller.hpp/.cpp
│   ├── inspector_controller.hpp/.cpp
│   ├── analysis_controller.hpp/.cpp
│   ├── project_controller.hpp/.cpp
│   └── export_controller.hpp/.cpp
│
├── panels/
│   ├── inspector_panel.hpp/.cpp
│   ├── analysis_control_bar.hpp/.cpp
│   └── results_dock_panel.hpp/.cpp
│
├── widgets/
│   ├── truss_canvas_widget.hpp/.cpp  # Core 2D rendering widget
│   ├── notification_rail.hpp/.cpp
│   └── analysis_options_dialog.hpp/.cpp
│
├── models/                           # Qt Item Models (read model layer)
│   ├── node_table_model.hpp/.cpp
│   ├── member_table_model.hpp/.cpp
│   ├── results_table_model.hpp/.cpp
│   └── validation_list_model.hpp/.cpp
│
├── state/
│   └── workspace_state.hpp           # WorkspaceState value object + WorkspacePhase enum
│
└── resources/
    ├── themes/
    │   ├── dark.qss
    │   └── light.qss
    ├── icons/                         # SVG icons (scalable, theme-adaptive via QIcon::fromTheme)
    └── resources.qrc
```

### 7.2 Namespacing Strategy

| Namespace           | Contents                                   |
| ------------------- | ------------------------------------------ |
| `truss::gui`        | All GUI classes (widgets, panels, dialogs) |
| `truss::gui::ctrl`  | All controller classes                     |
| `truss::gui::model` | All Qt Item Model classes                  |
| `truss::gui::state` | `WorkspaceState`, `WorkspacePhase`         |

### 7.3 Dependency Injection Approach

`main.cpp` (GUI entry point) is the **composition root**. It:

1. Constructs the concrete `TrussAnalysisFacade`
2. Passes it (as `ITrussAnalysisFacade&`) to `MainWindow`
3. `MainWindow` extracts `ITrussService*` and `IAnalysisService*` from the facade (via `static_cast` or accessor) for controller injection — or, more cleanly, `ITrussAnalysisFacade` is passed directly and controllers use it polymorphically

No controller contains a `new` call for any facade or service — all dependencies arrive through constructors.

```cpp
// main.cpp (composition root)
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Build the concrete facade (only place that names a concrete type)
    auto facade = truss::interface::TrussAnalysisFacade::create();

    truss::gui::MainWindow window(*facade);
    window.show();

    return app.exec();
}
```

### 7.4 Threading Strategy

```
Main Thread (Qt event loop)
│
├── All widget rendering
├── All signal/slot connections (except AnalysisWorker)
├── All facade reads (getTrussView, getResultsView) — immutable views, thread-safe
│
└── AnalysisWorker thread (QThread)
    │
    ├── Calls facade.analyze(truss, opts) — compute-bound, may take > 200ms
    └── Emits resultReady(ResultsHandle) via Qt::QueuedConnection back to main thread
```

**Rule**: The `AnalysisWorker` makes a **snapshot copy** of the `core::Truss` before the thread starts. It never holds a live reference to managed facade state during analysis.

```cpp
class AnalysisWorker : public QObject {
    Q_OBJECT
    core::Truss m_trussCopy;           // Value copy, immutable during analysis
    core::analysis::AnalysisOptions m_opts;
    truss::application::IAnalysisService* m_service;
public slots:
    void doAnalysis();
signals:
    void resultReady(truss::application::ResultsHandle handle);
    void failed(QString errorMessage);
};
```

### 7.5 Build System Integration

The GUI module is an existing CMake target (`TrussAnalysisGUI`). Changes:

1. Add `src/gui/panels/`, `src/gui/models/`, `src/gui/state/`, `src/gui/controllers/` source globs to `CMakeLists.txt`
2. Add `src/gui/resources/resources.qrc` to `qt_add_resources`
3. Ensure `TrussAnalysisGUI` links to `TrussInterface` (already in place) and `Qt6::Widgets`
4. Add `Qt6::Concurrent` if `QFuture`-based async is preferred over raw `QThread`

No changes to any other CMake target are required — the GUI is a leaf node in the dependency graph.

---

## 8. Validation Checklist

### 8.1 SOLID Principles

| Principle                 | Status | Evidence                                                                                                           |
| ------------------------- | ------ | ------------------------------------------------------------------------------------------------------------------ |
| **Single Responsibility** | PASS   | Each controller handles one workflow domain; each model handles one data type                                      |
| **Open/Closed**           | PASS   | New panels are added without modifying `MainWindow`; new export formats require only a new `ExportController` slot |
| **Liskov Substitution**   | PASS   | All controllers accept `ITrussService*` / `IAnalysisService*` — mock implementations are drop-in replacements      |
| **Interface Segregation** | PASS   | `CanvasController` receives `ITrussService*`, not `ITrussAnalysisFacade*` — sees only what it needs                |
| **Dependency Inversion**  | PASS   | No GUI class includes a concrete implementation header; all depend on `i*.hpp` interfaces                          |

### 8.2 DRY

| Area                | Status | Mechanism                                                                         |
| ------------------- | ------ | --------------------------------------------------------------------------------- |
| Colour definitions  | PASS   | All colours are QSS tokens — defined once, used everywhere                        |
| Error routing       | PASS   | All operation failures route through `NotificationRail::showError` via one signal |
| Table model refresh | PASS   | Single `refresh(TrussHandle)` method on each model — not duplicated per signal    |
| Facade call sites   | PASS   | Each facade method is called from exactly one controller                          |

### 8.3 YAGNI

| Potential Over-Engineering | Assessment                                                                                                                                    |
| -------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| Plugin system              | Not implemented — architecture supports it but does not build it                                                                              |
| 3D visualisation           | Not included — domain is strictly 2D                                                                                                          |
| Multi-document interface   | Not included — single truss per session matches domain scope                                                                                  |
| Undo/redo stack            | Not in v3.0.0 — `WorkspaceState.isDirty` and dirty-check banner provide adequate protection for now                                           |
| Material library server    | `MaterialLibraryService` already exists in the application layer — GUI uses it through the inspector combo box without any new infrastructure |

### 8.4 Clean Architecture

| Rule                              | Status | Evidence                                                                                                                                                                |
| --------------------------------- | ------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| No domain types in views          | PASS   | Views receive `ITrussView*` snapshots and `NodeView`/`MemberView` DTOs only                                                                                             |
| No UI code in domain              | PASS   | All Qt includes are confined to `src/gui/`                                                                                                                              |
| Dependency direction correct      | PASS   | GUI → Interface → Domain; never reversed                                                                                                                                |
| Facade is the only cross-boundary | PASS   | No view or controller includes any header from `src/core/` or `src/infrastructure/` except the view DTOs (`NodeView`, `MemberView`) which are part of the interface ABI |

### 8.5 Testability

| Component            | Test Approach                                                                      |
| -------------------- | ---------------------------------------------------------------------------------- |
| `CanvasController`   | Unit-tested with a mock `ITrussService`; verify correct facade calls on signals    |
| `AnalysisController` | Unit-tested with a mock `IAnalysisService`; use `QSignalSpy` to verify emission    |
| `NodeTableModel`     | Unit-tested by constructing with a mock `ITrussView`; verify `rowCount`, `data`    |
| `TrussCanvasWidget`  | Render-tested via `QTestLib` paint events with a known `ITrussView` fixture        |
| `MainWindow` (smoke) | Integration test: construct with mock facade, verify no crash on state transitions |

### 8.6 Maintainability

- All inter-component communication is through **named signals and slots** — dependencies are explicit and grep-able
- `WorkspaceState` is a value type — any panel can be removed without breaking the state machine
- QSS themes are self-contained — UI restyling requires zero C++ changes
- The `DisplayMode` enum is the single toggle point for canvas overlay variants

### 8.7 Extensibility

Future enhancements the architecture accommodates without structural refactoring:

| Enhancement                                        | Required Change                                                              |
| -------------------------------------------------- | ---------------------------------------------------------------------------- |
| New export format (e.g., PDF)                      | Add format to `ExportFormat` enum; add slot in `ExportController`            |
| Dynamic loading (add/remove members interactively) | `CanvasController` already mediates; extend `DrawingMode`                    |
| Undo / Redo                                        | Insert a `CommandStack` between `CanvasController` and facade calls          |
| Multi-load case                                    | New `LoadCaseSelector` panel; `AnalysisOptions` already carries options      |
| 3D extension                                       | Replace `TrussCanvasWidget` with an OpenGL widget; controllers are unchanged |
| Cloud sync / remote analysis                       | Replace the `AnalysisWorker` target URL; controllers are unchanged           |
| Accessibility audit                                | All widgets use `setAccessibleName`; QSS contrast is already WCAG-aligned    |

---

_End of Document_
