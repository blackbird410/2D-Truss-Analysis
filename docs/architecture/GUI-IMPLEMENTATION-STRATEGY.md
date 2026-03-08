# GUI Implementation Strategy
## 2D Truss Analysis — Execution-Ready Migration Plan

**Author:** Senior Software Architect & Delivery Strategist  
**Date:** March 2, 2026  
**Version Target:** 3.0.0  
**Prerequisite:** [GUI-MODERNIZATION-ARCHITECTURE.md](GUI-MODERNIZATION-ARCHITECTURE.md)

---

## Table of Contents

1. [Current Project State Assessment](#1-current-project-state-assessment)
2. [High-Level Migration Strategy](#2-high-level-migration-strategy)
3. [Phased Implementation Plan](#3-phased-implementation-plan)
4. [Testing Strategy](#4-testing-strategy)
5. [Integration Strategy](#5-integration-strategy)
6. [Risk Analysis & Mitigation Plan](#6-risk-analysis--mitigation-plan)
7. [Definition of Done](#7-definition-of-done)

---

## 1. Current Project State Assessment

### 1.1 Architectural Layer Summary

| Layer       | CMake Target       | Status   | Relevant To GUI Migration                                           |
| ----------- | ------------------ | -------- | ------------------------------------------------------------------- |
| Utility     | `TrussUtils`       | Stable   | `math_utils`, `string_utils` — no changes required                 |
| Core/Domain | `TrussCore`        | Stable   | View DTOs (`NodeView`, `MemberView`) consumed read-only by UI       |
| Application | `TrussApplication` | Stable   | `ITrussService`, `IAnalysisService`, `MaterialLibraryService`       |
| Interface   | `TrussInterface`   | Stable   | `ITrussAnalysisFacade` — sole dependency of new GUI                 |
| GUI         | `TrussAnalysisGUI` | Replace  | `MainWindow` + controllers + presenters + widgets — all replaced    |
| CLI         | `TrussAnalysisCLI` | Unchanged | Independent; migration does not touch CLI layer                    |

### 1.2 Integration Points Relevant to the New GUI

All GUI integration flows through one boundary:

```
src/interface/itruss_analysis_facade.hpp  →  ITrussAnalysisFacade
```

Supporting read-only view types that cross this boundary (part of interface ABI):

```
src/core/interfaces/itruss_view.hpp         → ITrussView, NodeView, MemberView
src/core/interfaces/ianalysis_results_view.hpp → IAnalysisResultsView
src/application/result.hpp                 → Result<T>, TrussHandle, ResultsHandle
src/application/truss_edit_dtos.hpp        → MaterialSpec, SectionSpec
src/core/model/types.hpp                   → NodeId, MemberId, Point2D, Force2D, SupportType
src/core/validation/truss_validator.hpp    → ValidationResult, ValidationIssue
```

### 1.3 Current GUI Implementation Facts

The existing `TrussAnalysisGUI` target (`src/gui/`) has the following characteristics:

- **`MainWindow`** constructor takes seven injected objects: `ITrussService&`,
  `IAnalysisService&`, plus three controllers and three presenters — assembled by
  `src/gui/main.cpp` after manually constructing all dependencies.
- **Controllers** (`AnalysisController`, `ProjectController`, `TrussEditController`)
  already depend on `ITrussService*` / `IAnalysisService*` interfaces and are unit-tested
  with `MockTrussApplicationService` / `MockAnalysisApplicationService`.
- **Presenters** (`AnalysisResultsPresenter`, `TrussDataPresenter`, `ValidationPresenter`)
  format domain view data into `QString`; they are simple pure functions and are already
  unit-tested.
- **Widgets** (`InteractiveDrawingWidget`, `ResultsWidget`, `DeformedTrussWidget`, plus
  five input form widgets) are tightly coupled to specific presenters and controller
  pairs; they are not independently testable.
- **No `WorkspaceState`**, no Qt Item Models, no `QDockWidget` results panel, no
  `NotificationRail`, no QSS theming.
- **Test coverage for GUI layer**:
  - 4 controller unit test files (GTest + GMock)
  - 1 presenter unit test file
  - 6 GUI integration test files (Qt Test + GTest)
  - `MockTrussAnalysisFacade` exists in `tests/mocks/`

### 1.4 Explicit Assumptions

1. `ITrussAnalysisFacade` interface is frozen — no modifications during this migration.
2. `TrussAnalysisFacade::create()` factory method exists (as referenced in the architecture
   document); if not, `main.cpp` must construct the facade inline.
3. Qt6 `Widgets` and `Test` are already present in the build environment.
4. The build flag `BUILD_GUI` remains the compile switch for all GUI code.
5. The CLI target is not affected by any change in this plan.
6. GTest + GMock remain the testing frameworks for all new controller tests.

### 1.5 Technical Constraints

| Constraint                              | Impact                                                             |
| --------------------------------------- | ------------------------------------------------------------------ |
| C++20; no QML                           | All new files are `.hpp`/`.cpp` Qt Widgets; no `.qml` files       |
| Qt6 `AUTOMOC ON` already set            | `Q_OBJECT` classes compile correctly without manual moc invocation |
| `POSITION_INDEPENDENT_CODE ON` required | New static library targets must carry this property                |
| `tests/mocks/` already contains `MockTrussAnalysisFacade` | Re-use; do not create a second mock |
| Integration tests include GUI source files directly (not via library) | New panels/models must be added to the CMake `list(APPEND ...)` blocks for test targets |

### 1.6 Technical Risks (Summary — detailed in Section 6)

- The existing `MainWindow` constructor is wired in `src/gui/main.cpp`; removing it
  while integration tests still reference it will cause link failures.
- GUI integration tests depend on the concrete `AnalysisController`,
  `ProjectController`, and `TrussEditController` via `list(APPEND ...)` — any rename
  breaks the test build.
- `InteractiveDrawingWidget` is 387+ lines with embedded coordinate transformation,
  tool state, and signal emission logic — it is the highest-risk component to replace.

---

## 2. High-Level Migration Strategy

### 2.1 Chosen Approach: Incremental Replacement with Parallel Compilation

**Big-bang replacement is rejected.** Reasons:

1. Six GUI integration test files already pass against the current controllers and
   presenters. Deleting these before replacements exist eliminates regression coverage
   during the most dangerous phase.
2. The `MockTrussAnalysisFacade` and four controller mock-based test suites represent
   working test infrastructure. This is preserved and extended, not discarded.
3. The interface layer (`ITrussAnalysisFacade`) is stable. The new and old GUI surfaces
   can coexist in the CMake target simultaneously.

**Strategy: stacked component replacement.**

New components are built alongside existing ones. Each new component is unit-tested
before the old counterpart is deleted. The `MainWindow` is replaced last, at which
point all its constituent parts have verified replacements.

### 2.2 Feature Flag / Parallel-Run Considerations

No runtime feature flag is required. The transition is tracked at the file level — old
files exist until deleted in Phase 8. The CMake `BUILD_GUI` flag is the single compile
boundary. No runtime toggle is introduced; YAGNI applies.

### 2.3 Legacy Decommissioning Strategy

Decommissioning proceeds in this order (strict):

1. Input form widgets (`NodeInputWidget`, `MemberInputWidget`, `LoadInputWidget`,
   `DataTableWidget`) — replaced by `InspectorPanel` sections.
2. Presenters (`AnalysisResultsPresenter`, `TrussDataPresenter`, `ValidationPresenter`)
   — replaced by Qt Item Models.
3. `InteractiveDrawingWidget`, `ResultsWidget`, `DeformedTrussWidget`, `PlotWidget`
   — replaced by `TrussCanvasWidget` and `ResultsDockPanel`.
4. Controllers (`AnalysisController`, `ProjectController`, `TrussEditController`)
   — replaced by new controllers in `src/gui/controllers/`.
5. `MainWindow` — replaced last once all constituent replacements are verified.
6. `src/gui/main.cpp` composition root — rewritten, not deleted.

Each deletion is a discrete commit. No file is deleted and replaced in the same commit.

### 2.4 Backward Compatibility

There are no backward compatibility requirements for the GUI binary. The CLI binary,
the interface layer, and all lower layers are unaffected. The JSON/XML project file
format is unchanged.

---

## 3. Phased Implementation Plan

### Phase 1 — Project Structure Preparation

**Objective:** Create the new directory structure and CMake scaffolding without changing
any existing code. Confirm the build produces no new warnings.

**Scope:**

- Directory creation only: `src/gui/panels/`, `src/gui/models/`, `src/gui/state/`,
  `src/gui/resources/themes/`, `src/gui/resources/icons/`
- Stub header files for every new component (class declaration, no implementation,
  documented interface)
- `workspace_state.hpp` — `WorkspaceState` and `WorkspacePhase` definition (value
  types, no Qt dependency)
- CMake: add new `src/gui/panels/`, `src/gui/models/`, `src/gui/state/` source
  directories to `TrussAnalysisGUI` target placeholders

**Technical Tasks:**

1. Create directory tree as specified in Section 7.1 of the architecture document.
2. Write `src/gui/state/workspace_state.hpp` — complete definition, no `.cpp` needed.
3. Write stub header for each of: `TrussCanvasWidget`, `InspectorPanel`,
   `AnalysisControlBar`, `ResultsDockPanel`, `NotificationRail`,
   `AnalysisOptionsDialog`.
4. Write stub headers for controllers: `MainWindowController`, `CanvasController`,
   `InspectorController`, `AnalysisController` (new), `ProjectController` (new),
   `ExportController`.
5. Write stub headers for models: `NodeTableModel`, `MemberTableModel`,
   `ValidationListModel`, `ResultsTableModel`.
6. Create `src/gui/resources/resources.qrc` (empty, to be populated in Phase 7).
7. Verify `cmake --build build` is clean with the new stub files added to
   `TrussAnalysisGUI` source list.

**Dependencies:** None.

**Exit Criteria:**

- Build succeeds with zero new errors or warnings.
- All new directories and stub files exist.
- `WorkspaceState` compiles as a standalone header.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| Stub headers include Qt types that trigger MOC re-runs on every build | Do not include `Q_OBJECT` in stubs; add it only when the class body is implemented |
| CMake glob patterns silently miss new files | Use explicit file lists in CMakeLists.txt, not `GLOB` |

---

### Phase 2 — Qt Infrastructure Setup

**Objective:** Implement the state management foundation and QSS theming system.
These have zero coupling to domain logic and can be built and tested immediately.

**Scope:**

- `WorkspaceState` — complete (already done in Phase 1)
- QSS theme files: `dark.qss`, `light.qss`
- Theme loader utility function
- `NotificationRail` — full implementation

**Technical Tasks:**

1. Write `dark.qss` and `light.qss` using the colour tokens defined in the architecture
   document (Section 5.4.1). All widget selectors use `objectName` targeting.
2. Write `resources.qrc` entries for both theme files and placeholder icon directory.
3. Implement `NotificationRail`:
   - `QPropertyAnimation` for slide-in (150ms, `maximumHeight` property)
   - Auto-dismiss `QTimer` (4000ms for Info/Success; disabled for Error)
   - Stack up to 3 simultaneous notifications via `QVBoxLayout`
   - `showInfo(QString)`, `showSuccess(QString)`, `showWarning(QString)`,
     `showError(QString)` public slots
4. Theme loader: `void applyTheme(QApplication&, const QString& themeResourcePath)` —
   reads QSS from Qt resource, calls `app.setStyleSheet(...)`. Persists selection to
   `QSettings("TrussAnalysis", "GUI")`.

**Dependencies:** Phase 1 (directory structure).

**Exit Criteria:**

- `NotificationRail` unit test passes: verify all four severity types show and
  auto-dismiss correctly using `QSignalSpy` and `QTest::qWait`.
- Both QSS files compile into the resource binary without error.
- Theme loader function applies styles correctly in a minimal `QApplication` test.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| `QPropertyAnimation` on `maximumHeight` causes layout jitter on macOS | Test on target platform (Linux) early; use `setFixedHeight` fallback if needed |
| QSS selectors too broad, unintentionally style Qt dialogs | Scope all selectors to named widgets; prefix with `QMainWindow` root where possible |

---

### Phase 3 — Qt Item Models

**Objective:** Implement the read-model layer that bridges domain view data to Qt's
MVC system. These models have no facade access and are testable in complete isolation.

**Scope:**

- `NodeTableModel` — full implementation
- `MemberTableModel` — full implementation
- `ValidationListModel` — full implementation
- `ResultsTableModel` — skeleton (populated in Phase 6)

**Technical Tasks:**

1. **`NodeTableModel`** derives `QAbstractTableModel`:
   - `void refresh(const core::interfaces::ITrussView& view)` — calls
     `beginResetModel()` / `endResetModel()`, copies `getNodeViews()` into
     `std::vector<NodeView> m_rows`
   - Columns: ID, X, Y, Support, Fx, Fy, dx, dy, Rx, Ry (10 columns)
   - `data(Qt::DisplayRole)` — format floats with `QLocale().toString(v, 'g', 6)`
   - `data(Qt::BackgroundRole)` — amber highlight when `|fx| > 0 || |fy| > 0`
   - `data(Qt::ToolTipRole)` — support type description
   - Pre-analysis columns (dx, dy, Rx, Ry) display `"—"` until `setHasResults(true)`

2. **`MemberTableModel`** derives `QAbstractTableModel`:
   - 12 columns: ID, Start Node, End Node, Material, E [GPa], A [cm²], Length [m],
     Angle [°], Axial Force [kN], Stress [MPa], Utilisation Ratio, State
   - `data(Qt::ForegroundRole)` for Ratio column: linear interpolation
     green (#34A853) → amber (#FFC107) → red (#EA4335) at 0.0 / 0.75 / 1.0
   - `data(Qt::ToolTipRole)` for State column: "Tension" / "Compression" / "Yielded"
   - Pre-analysis columns display `"—"`

3. **`ValidationListModel`** derives `QAbstractListModel`:
   - `void refresh(const core::validation::ValidationResult& result)` — stores
     `std::vector<ValidationIssue> m_issues`
   - `data(Qt::DisplayRole)` — `"[CATEGORY] message"`
   - `data(Qt::DecorationRole)` — severity icon from resource: `:/icons/severity_*.svg`
   - `data(Qt::UserRole + 1)` — `QVariant` holding first affected `NodeId` (for
     click-to-select, consumed by `InspectorController`)
   - `data(Qt::UserRole + 2)` — `QVariant` holding first affected `MemberId`

4. **`ResultsTableModel`** — skeleton only: `rowCount` returns 0; `columnCount` returns
   the system-summary key count; populated fully in Phase 6.

**Dependencies:** Phase 1 (stubs exist so compilation units are valid).

**Exit Criteria:**

- All three models pass unit tests with a manually constructed `ITrussView` test fixture.
- `NodeTableModel::refresh()` with a 3-node truss produces `rowCount() == 3`.
- `MemberTableModel` colour delegate returns correct `QColor` for ratio = 0.5
  (expected: midpoint between green and amber).
- Unit tests compile and run in under 2 seconds (no Qt event loop required).

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| `beginResetModel` / `endResetModel` cycle loses selection in connected `QTableView` | Acceptable for v3.0.0; incremental refresh can be added later per YAGNI |
| `ITrussView` returns `std::vector<NodeView>` by value on each call — expensive for large models | Cache the vector in `m_rows` after `refresh()`; do not call `getNodeViews()` in `data()` |

---

### Phase 4 — Core Window & Navigation Framework

**Objective:** Implement `TrussCanvasWidget` (rendering only, no interaction yet) and
the skeleton `MainWindow` that hosts the new layout structure. The legacy `MainWindow`
remains on disk and compiles in parallel.

**Scope:**

- `TrussCanvasWidget` — rendering pipeline only (no interaction)
- New `MainWindow` skeleton with `QSplitter`, `QDockWidget`, menu bar, tool strip stubs
- `MainWindowController` — construction and `WorkspaceState` ownership

**Technical Tasks:**

1. **`TrussCanvasWidget`** — `paintEvent` pipeline (7 ordered steps from architecture
   Section 6.2):
   - `QTransform m_worldToScreen` — initialised as identity; updated by `setViewport()`
   - `void refresh(const core::interfaces::ITrussView* view, DisplayMode mode)` — null
     view draws empty canvas with "No Model Loaded" centred text
   - Node rendering: `QPainter` filled circle (radius 6px in screen space), colour by
     support type; support symbol drawn at ×1.5 radius
   - Member rendering: line from `worldToScreen(startNode)` to `worldToScreen(endNode)`;
     colour per `DisplayMode` and member state
   - Force arrow rendering: arrow from node centre in force direction, scaled to max
     force magnitude; label with magnitude
   - Deformed shape: ghost (30% opacity) original + coloured deformed overlay; scale
     factor from settings or auto-fit
   - Grid: optional 1m grid lines using `--bg-secondary` pen at 1px width
   - No interaction signals in this phase

2. **New `MainWindow` skeleton**:
   - Constructor takes only `ITrussAnalysisFacade& facade`
   - `setupLayout()`: `QSplitter` (horizontal) as central widget; left = canvas
     placeholder `QWidget`; right = inspector placeholder `QWidget`
   - `QDockWidget` at `Qt::BottomDockWidgetArea` with tab placeholder
   - Menu bar: File, Edit, View, Analysis, Export, Help (actions are stubs that
     `qDebug()` until wired in Phase 5)
   - Toolbar: `QToolBar` with `QAction` objects for all 9 toolbar items; icons loaded
     from resource; actions disabled until wired

3. **`MainWindowController`**:
   - Holds `ITrussAnalysisFacade* m_facade`
   - Holds `WorkspaceState m_state`
   - `void setState(WorkspaceState newState)` — emits `stateChanged(WorkspaceState)`
   - `void onTrussModified(application::TrussHandle handle)` — updates `m_state.isDirty`
     and `m_state.trussHandle`, calls `setState()`
   - `void onAnalysisCompleted(application::ResultsHandle rh)` — updates phase to
     `ResultsReady`

**Dependencies:** Phases 1, 2, 3.

**Exit Criteria:**

- `TrussCanvasWidget` renders a 3-member test truss correctly with no assertion failures
  in a `QTestLib` paint test.
- New `MainWindow` opens and displays without crash when passed a `MockTrussAnalysisFacade`.
- `stateChanged` signal fires correctly when `MainWindowController::setState()` is called,
  verified with `QSignalSpy`.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| Two `MainWindow` classes in the same CMake target cause symbol conflicts | Keep old `main_window.cpp` in the `TrussAnalysisGUI` source list; rename new file `main_window_v2.cpp` (temporary) until old one is deleted in Phase 8 |
| `QPainter` coordinate space mismatch (Y-axis flipped in Qt vs. structural engineering) | Apply `m_worldToScreen.scale(1, -1)` and translate origin to canvas bottom-left; document this transform clearly |

---

### Phase 5 — Feature Module Implementation

**Objective:** Implement all secondary panels and their controllers with full signal/slot
wiring. Each component is independently unit-tested before being assembled.

**Scope:**

- `InspectorPanel` (4 stacked sections)
- `AnalysisControlBar`
- `ResultsDockPanel` (tabs with `QTableView` bindings to Phase 3 models)
- `AnalysisOptionsDialog`
- `CanvasController`, `InspectorController`, `AnalysisController` (new),
  `ProjectController` (new), `ExportController`

**Technical Tasks:**

1. **`InspectorPanel`**:
   - `QStackedWidget` with 4 pages managed by index constants:
     `kPageProject = 0`, `kPageNode = 1`, `kPageMember = 2`, `kPageValidation = 2`  
     (Note: index 3 for validation)
   - Page 0 (Project Summary): `QFormLayout` with project name `QLineEdit` (read-only),
     node count, member count, DOF count — refreshed from `WorkspaceState`
   - Page 1 (Node Editor): X `QLabel` (read-only), Y `QLabel` (read-only), support
     `QComboBox` (5 items: Free/Pinned/RollerX/RollerY/Fixed), Fx/Fy `QDoubleSpinBox`
     (range ±1e12 N, suffix " N"); "Apply" button emits
     `loadChangeRequested(NodeId, Force2D)`
   - Page 2 (Member Editor): material `QComboBox` (populated from
     `MaterialLibraryService`), area `QDoubleSpinBox` (range 1e-6 to 1.0 m², suffix
     " m²"), read-only labels: length, angle, axial force, stress, utilisation ratio;
     "Apply" button emits `memberPropertyChanged(MemberId, MaterialSpec, SectionSpec)`
   - Page 3 (Validation Summary): `QListView` bound to `ValidationListModel`; click
     selects element on canvas via `InspectorController`

2. **`AnalysisControlBar`**:
   - `QPushButton m_runButton` — icon + "Run Analysis"; transitions to
     `QProgressBar` (indeterminate) during `Analysing` phase by hiding button and
     showing progress bar in same `QStackedWidget`
   - `QPushButton m_validateButton` — "Validate"
   - `QToolButton m_optionsButton` — gear icon; click opens `AnalysisOptionsDialog`
   - Both buttons disabled when `phase == Analysing`
   - `onStateChanged(WorkspaceState)` enables/disables buttons

3. **`ResultsDockPanel`**:
   - Title: "Results"
   - `QTabWidget` with 4 tabs
   - Tab 0 (Node Results): `QTableView` → `NodeTableModel`
   - Tab 1 (Member Results): `QTableView` → `MemberTableModel` + `UtilisationDelegate`
     — a `QStyledItemDelegate` that paints column 10 (Ratio) as a colour-filled bar
   - Tab 2 (System Summary): `QFormLayout` — max displacement, max stress, converged
     (✓/✗), iterations, condition number, free DOFs, constrained DOFs, total strain
   - Tab 3 (Stiffness Matrix): `QTableWidget` — populated lazily only when tab is
     active (connect `QTabBar::tabBarClicked(int)` to `onStiffnessTabSelected()`)
   - All tabs show placeholder text "Run analysis to see results" when
     `ResultsHandle == 0` via `QStackedWidget`
   - "Export" `QPushButton` per tab; emits `exportRequested(format, suggestedFilename)`

4. **Controller implementations** — each derives `QObject`:

   **`CanvasController`**:
   - Receives `ITrussService*` in constructor
   - `onNodeDropRequested(Point2D, SupportType)` → `facade->addNode(...)` →
     on success emit `trussModified(TrussHandle)`, on failure emit `operationFailed(QString)`
   - `onMemberDrawRequested(NodeId, NodeId)` → `facade->addMember(...)`
   - `onDeleteRequested(NodeId)` → `facade->removeNode(...)` (guard: if node has
     connected members, emit `operationFailed` with explanation)
   - `onDeleteRequested(MemberId)` → `facade->removeMember(...)`

   **`InspectorController`**:
   - Receives `ITrussService*` in constructor; holds current `TrussHandle`
   - `onSelectionChanged(NodeId id)` → queries `facade->getTrussView(handle)` →
     finds `NodeView` by id → calls `m_panel->showNodeEditor(nodeView)`
   - `onSupportChangeRequested(NodeId, SupportType)` → `facade->setNodeSupport(...)`
   - `onLoadChangeRequested(NodeId, Force2D)` → `facade->applyNodeLoad(...)`

   **`AnalysisController` (new)**:
   - Receives `IAnalysisService*`, `ITrussService*` in constructor
   - Constructs `AnalysisWorker`, moves to `QThread m_workerThread`
   - `onAnalyzeRequested(AnalysisOptions opts)`:
     1. Query `facade->getTrussMutable(handle)` — copy to `m_worker->m_trussCopy`
     2. Set `m_worker->m_opts`
     3. Emit `analysisStarted()`
     4. Invoke `QMetaObject::invokeMethod(m_worker, "doAnalysis", Qt::QueuedConnection)`
   - `onWorkerResultReady(ResultsHandle)` → emit `analysisCompleted(ResultsHandle)`
   - `onWorkerFailed(QString)` → emit `analysisFailed(QString)`

   **`ProjectController` (new)**:
   - `onNewProjectRequested()` — if `state.isDirty` show `QMessageBox::question`
     (one exception to no-modal rule: project destruction is irreversible); else
     `facade->clearAll()` → emit `trussCreated(TrussHandle)`
   - `onOpenFileRequested()` → `QFileDialog::getOpenFileName` → `facade->loadTruss(path)`
   - `onSaveRequested()` → if path known, `facade->saveTruss(h, path, true)`;
     else delegate to `onSaveAsRequested()`
   - `onSaveAsRequested()` → `QFileDialog::getSaveFileName` → `facade->saveTruss(...)`

   **`ExportController`**:
   - `onExportRequested(ResultsHandle, ExportFormat, path)` → `facade->exportResults(...)`
   - On success emit `exportCompleted(path)`; on failure emit `exportFailed(QString)`

**Dependencies:** Phases 3, 4.

**Exit Criteria:**

- Each controller unit test passes with `MockTrussAnalysisFacade`; verify exact facade
  method is called with correct arguments using `EXPECT_CALL`.
- `InspectorPanel` context switching works: `QSignalSpy` on `selectionChanged` verifies
  `QStackedWidget::currentIndex()` changes to the correct page.
- `ResultsDockPanel` stiffness matrix tab triggers lazy population only on first click.
- `AnalysisController` emits `analysisStarted()` before worker starts and
  `analysisCompleted(handle)` after `QThread` finishes, verified with `QSignalSpy` in
  a `QTestLib` test that uses a mock `IAnalysisService` with fixed return value.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| `AnalysisWorker` accesses `IAnalysisService*` from a non-main thread — undefined if service has main-thread affinity | `AnalysisWorker::doAnalysis()` must only call `IAnalysisService::analyze(const Truss&, opts)` using the value-copy; never call `getTrussView` or `getTrussMutable` from the worker thread |
| `ProjectController::onNewProjectRequested` uses `QMessageBox` — breaks headless tests | Introduce `IConfirmationProvider` interface with `virtual bool confirm(QString msg)` defaulting to modal; swap in a `AutoConfirmProvider` in tests |

---

### Phase 6 — State Management & Interaction Logic

**Objective:** Wire all components into `MainWindowController` and the new `MainWindow`.
Connect all signals and slots. Implement full canvas interaction (pan, zoom, draw tools).
Replace the composition root in `src/gui/main.cpp`.

**Scope:**

- Full canvas interaction: pan, zoom, node drop, member draw, select, delete
- Complete signal/slot wiring across all controllers and panels
- New `MainWindow` — full implementation replacing the skeleton from Phase 4
- `src/gui/main.cpp` — rewritten as clean composition root
- `WorkspaceState` state machine: all phase transitions

**Technical Tasks:**

1. **`TrussCanvasWidget` — interaction layer**:
   - `mousePressEvent`:
     - `AddNode` mode → snap to grid if enabled → emit `nodeDropRequested(pos, SupportType::Free)`
     - `AddMember` mode → if no start node captured, find nearest node within 12 screen-px
       → record as `m_pendingMemberStart`; on second click, emit
       `memberDrawRequested(startId, endId)`
     - `Select` mode → hit-test nodes (12px radius) then members (4px perpendicular
       distance) → emit `selectionChanged(NodeId)` or `selectionChanged(MemberId)`
   - `mouseMoveEvent`:
     - Pan: if middle-button held, update `m_panOffset` and `call update()`
     - `AddMember` mode: update ghost preview line endpoint → `call update()`
     - Emit `cursorPositionChanged(Point2D worldPos)` for status bar coordinate display
   - `wheelEvent`: zoom around cursor position by updating `m_scale` and `m_panOffset`
     to maintain cursor position in world space
   - `keyPressEvent`: `Esc` → `DrawingMode::Select`; `N` → `AddNode`;
     `M` → `AddMember`; `Delete` → emit `deleteRequested`

2. **`MainWindowController` — full wiring**:
   - `onTrussModified(TrussHandle)`:
     1. Query `m_facade->getTrussView(handle)`
     2. Refresh `m_nodeModel->refresh(view)`, `m_memberModel->refresh(view)`
     3. Refresh canvas: `m_canvas->refresh(&view, m_displayMode)`
     4. Call `m_facade->validateTruss(handle)` → refresh `m_validationModel->refresh(result)`
     5. Update phase to `ModelBuilding`; set `isDirty = true`
   - `onAnalysisCompleted(ResultsHandle rh)`:
     1. Query `m_facade->getResultsView(rh)`
     2. Refresh all models with results
     3. Re-refresh canvas with results overlay
     4. Update phase to `ResultsReady`; `resultsHandle = rh`

3. **New `MainWindow` — full implementation**:
   - Construct `MainWindowController` with `facade`
   - Construct all controllers, pass sub-interfaces extracted from facade (via
     `dynamic_cast<ITrussService*>(&facade)` or facade stores and exposes them)
   - Assemble widget hierarchy: `QSplitter` → `[TrussCanvasWidget | InspectorPanel]`;
     `QDockWidget` → `ResultsDockPanel`
   - Insert `NotificationRail` as a fixed-height widget below toolbar in the central
     area's layout (not inside the splitter)
   - Connect all signals as documented in architecture Section 5.6
   - `closeEvent`: check `WorkspaceState.isDirty` → show confirm dialog if unsaved

4. **Composition root rewrite** (`src/gui/main.cpp`):
   ```cpp
   int main(int argc, char* argv[]) {
       QApplication app(argc, argv);
       app.setOrganizationName("TrussAnalysis");
       app.setApplicationName("TrussAnalysisGUI");

       applyTheme(app, loadPersistedTheme());

       auto facade = std::make_unique<truss::interface::TrussAnalysisFacade>();
       truss::gui::MainWindow window(*facade);
       window.show();
       return app.exec();
   }
   ```

**Dependencies:** Phases 4, 5.

**Exit Criteria:**

- End-to-end workflow test passes (headless using `QTestLib` + `MockTrussAnalysisFacade`):
  1. Simulate `CanvasWidget` node-drop signal
  2. Verify `CanvasController` calls `facade->addNode`
  3. Verify `MainWindowController` emits `stateChanged` with `isDirty == true`
  4. Verify `NodeTableModel::rowCount()` increments
- Canvas zoom: verify `QTransform` scale factor increases by ~1.15 per wheel step.
- `WorkspacePhase` transitions correctly through all states in a scripted test sequence.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| Canvas hit-testing inaccurate after pan + zoom because screen-to-world inverse is not applied | Implement `Point2D screenToWorld(QPoint p)` using `m_worldToScreen.inverted()` and use it in all event handlers |
| `validateTruss` called on every edit creates performance regression for large models | Add debounce: `QTimer::singleShot(200ms, ...)` to coalesce rapid edits before triggering validation |

---

### Phase 7 — Styling & UI Consistency Enforcement

**Objective:** Apply QSS themes to all new widgets, add SVG icons, enforce keyboard
navigation, and validate WCAG colour contrast.

**Scope:**

- All new widgets assigned `objectName` strings matching QSS selectors
- `dark.qss` and `light.qss` complete and tested
- SVG icon set integrated into `resources.qrc`
- View/Analysis menu updated with theme toggle action
- Keyboard navigation verified for all panels

**Technical Tasks:**

1. Assign `setObjectName()` to all new widgets during construction. Naming convention:
   `"panelName_widgetRole"` e.g., `"inspector_supportCombo"`.

2. Complete both QSS files to style all named widgets. Cover at minimum:
   `QPushButton`, `QComboBox`, `QDoubleSpinBox`, `QTableView`, `QDockWidget`,
   `QToolBar`, `QStatusBar`, `QGroupBox`, `QLabel[role="sectionTitle"]`.

3. Source or create SVG icons for the 9 toolbar actions. Minimum size: 24×24px.
   Store in `src/gui/resources/icons/`. Update `resources.qrc`.

4. Add "View → Theme → Dark / Light" menu actions wired to `applyTheme()`.
   Persist selection via `QSettings`.

5. Verify tab-order navigation through `InspectorPanel` form fields using
   `QWidget::setTabOrder()` chaining.

6. Verify colour contrast ratios for primary text on primary background meet WCAG AA
   (minimum 4.5:1). Tool: contrast ratio calculation against defined tokens.
   - Dark theme: `#E8EAED` on `#1E2028` = 12.2:1 ✓
   - Light theme: `#1A1A1A` on `#F5F5F5` = 16.8:1 ✓

**Dependencies:** Phases 4, 5, 6.

**Exit Criteria:**

- Both themes apply without any unstyled ("native") widget visible in a visual
  inspection run on Linux (target platform).
- All 9 toolbar actions have icons.
- Theme selection persists across application restart.
- Full keyboard navigation through InspectorPanel fields in correct tab order.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| QSS `QDockWidget` title bar styling inconsistent across Qt6 minor versions | Test on Qt 6.4+ explicitly; use `QDockWidget::setTitleBarWidget()` with a custom `QWidget` if native styling is inadequate |
| SVG icon rendering quality on high-DPI (macOS dev machine differs from Linux target) | Use `@2x` variants in resource file or `QIcon::fromTheme` fallbacks; test on actual Linux display |

---

### Phase 8 — Legacy UI Decommissioning

**Objective:** Delete all legacy GUI source files, remove their test references, and
clean up the CMakeLists.txt. No functionality is removed — only redundant implementations.

**Scope:** All files in `src/gui/` that are not part of the new architecture.

**Technical Tasks (in strict order):**

1. Delete input form widgets (5 files):
   `node_input_widget`, `member_input_widget`, `load_input_widget`,
   `data_table_widget`, `plot_widget`
   Update `TrussAnalysisGUI` CMake source list.

2. Delete presenters (3 files):
   `analysis_results_presenter`, `truss_data_presenter`, `validation_presenter`
   — Remove from CMake source lists for `TrussAnalysisGUI`, `unit_tests`,
   `integration_tests`, `gui_integration_test`.

3. Delete old widgets (3 files):
   `interactive_drawing_widget`, `results_widget`, `deformed_truss_widget`

4. Delete old controllers (3 files):
   `analysis_controller`, `project_controller`, `truss_edit_controller`
   — Remove from all CMake source lists.
   — Delete corresponding test files from `tests/unit/gui/controllers/`:
     `test_truss_edit_controller.cpp`, `test_truss_edit_controller_advanced.cpp`,
     `test_project_controller.cpp`, `test_analysis_controller.cpp`
     (their new counterparts under `tests/unit/gui/controllers/` replace them)

5. Delete presenter test file:
   `tests/unit/gui/presenters/test_truss_data_presenter.cpp`

6. Delete old `main_window.cpp` / `main_window.hpp` (legacy).
   Rename `main_window_v2.cpp` → `main_window.cpp` (from Phase 4 temporary name).

7. Rebuild from clean: `cmake --build build --target TrussAnalysisGUI`.
   All integration tests must pass.

**Dependencies:** Phase 6 complete and all new controller unit tests passing.

**Exit Criteria:**

- `src/gui/` contains only files defined in architecture document Section 7.1.
- `cmake --build` produces no warnings about missing source files.
- All test suites pass with no skipped tests.
- No references to deleted class names remain in any `.cpp` or `.hpp` file.

**Risks & Mitigation:**

| Risk | Mitigation |
| ---- | ---------- |
| Old controller test suite covers behaviour not yet covered by new tests | Audit old test cases before deletion; port unique scenarios to new test files |
| Integration test `test_gui_integration.cpp` depends on deleted presenter files | Replace with a new `test_gui_integration.cpp` that tests the new controller/panel signal wiring in Phase 6 |
| CMake build cache retains stale object files after source deletion | Run `cmake --build build --clean-first` after all deletions |

---

### Phase 9 — Final Integration & Hardening

**Objective:** Verify production readiness. Address any issues found in full system
integration testing. Update all documentation.

**Scope:**

- Full test suite execution
- Performance validation
- Documentation updates
- Final code review

**Technical Tasks:**

1. Run complete test suite: `ctest --test-dir build -V`. All tests must pass.
2. Run GUI smoke test on Linux: open application, load `examples/bridge-truss/`,
   run analysis, verify canvas overlay, export to CSV, verify file.
3. Run `clang-tidy` on all new `src/gui/` files using project's existing
   `StaticAnalysis.cmake` configuration.
4. Run `clang-format` on all new files.
5. Verify `TrussAnalysisCLI` still builds and all CLI tests pass (regression check).
6. Update `GUI-MODERNIZATION-ARCHITECTURE.md` status fields from "Design Proposal" to
   "Implemented".
7. Update `README.md` to reflect new GUI capabilities and keyboard shortcuts.
8. Close any TODO/FIXME comments introduced during earlier phases.

**Dependencies:** Phase 8 complete.

**Exit Criteria:** See [Section 7 — Definition of Done](#7-definition-of-done).

---

## 4. Testing Strategy

### 4.1 Unit Testing Scope

All new unit tests live in `tests/unit/gui/` mirroring the source tree:

```
tests/unit/gui/
├── controllers/
│   ├── test_canvas_controller.cpp
│   ├── test_inspector_controller.cpp
│   ├── test_analysis_controller.cpp
│   ├── test_project_controller.cpp
│   └── test_export_controller.cpp
├── models/
│   ├── test_node_table_model.cpp
│   ├── test_member_table_model.cpp
│   └── test_validation_list_model.cpp
└── state/
    └── test_workspace_state.cpp
```

**Unit test scope per component:**

| Component            | What is tested                                                                   |
| -------------------- | -------------------------------------------------------------------------------- |
| `CanvasController`   | `addNode` success path calls `facade->addNode`; failure path emits `operationFailed`; `addMember` calls `facade->addMember` |
| `InspectorController`| `onSelectionChanged(NodeId)` calls `getTrussView` and emits correct panel-switch  |
| `AnalysisController` | `onAnalyzeRequested` emits `analysisStarted`; mock worker result triggers `analysisCompleted` |
| `ProjectController`  | `onOpenFileRequested` calls `facade->loadTruss`; dirty-state guard prevents clear without confirm |
| `ExportController`   | `onExportRequested` calls `facade->exportResults`; failure path emits `exportFailed` |
| `NodeTableModel`     | `rowCount()` after `refresh(3-node view) == 3`; `data(BackgroundRole)` for loaded node returns amber colour |
| `MemberTableModel`   | Pre-analysis ratio column returns `"—"`; post-analysis returns formatted value with correct colour |
| `ValidationListModel`| `rowCount()` matches `ValidationResult::getIssues().size()`; `DecorationRole` differs by severity |
| `WorkspaceState`     | Value semantics: copy/assign, equality, default construction                      |

All controller unit tests use `MockTrussAnalysisFacade` from `tests/mocks/`.

### 4.2 Integration Testing Boundaries

Integration tests verify cross-component signal/slot chains. They use `QTestLib` with
an event loop where required:

| Test File                              | Boundary Tested                                                 |
| -------------------------------------- | --------------------------------------------------------------- |
| `test_gui_integration.cpp` (new)       | Canvas signal → controller → mock facade → `stateChanged` emission |
| `test_gui_lifecycle.cpp` (update)      | `MainWindow` construction with `MockTrussAnalysisFacade` → no crash |
| `test_e2e_workflows.cpp` (update)      | Full workflow: load file → validate → analyze → export using real facade |

### 4.3 Mocking Strategy for `ITrussAnalysisFacade`

`tests/mocks/mock_truss_analysis_facade.hpp` already contains `MockTrussAnalysisFacade`
covering all `ITrussAnalysisFacade` methods. **Do not create a second mock.**

For widget-level tests where only `ITrussService` methods are needed, use
`MockTrussApplicationService` from `tests/mocks/mock_truss_application_service.hpp`.

Standard mock usage pattern for controller tests:

```cpp
class CanvasControllerTest : public ::testing::Test {
protected:
    truss::test::MockTrussAnalysisFacade mockFacade;
    truss::gui::ctrl::CanvasController controller{&mockFacade};
};

TEST_F(CanvasControllerTest, AddNodeCallsFacadeOnSuccess) {
    EXPECT_CALL(mockFacade, addNode(_, _, _))
        .WillOnce(Return(Result<NodeId>::Success(NodeId{1})));

    QSignalSpy spy(&controller, &CanvasController::trussModified);
    controller.onNodeDropRequested({1.0, 2.0}, SupportType::Free);

    ASSERT_EQ(spy.count(), 1);
}
```

### 4.4 UI Testing Approach

All GUI component tests use `QTestLib` (`Qt6::Test`). They are compiled into the
`unit_tests` binary under the `BUILD_GUI` conditional block in CMakeLists.txt.

No external UI automation framework (e.g., Squish) is introduced. YAGNI: the Qt Test
`QSignalSpy` + `QTest::mouseClick` / `QTest::keyClick` set is sufficient for v3.0.0.

`TrussCanvasWidget` render tests use `QTest::grabWidget()` to capture a `QPixmap` and
verify specific pixel colours at known coordinates for a fixed test truss. This is
fragile for fine-grained layout changes but provides a smoke-test safety net.

### 4.5 Regression Testing Plan

The following test sets must all pass before Phase 8 (legacy deletion) proceeds:

1. `unit_tests` — all 80+ existing non-GUI tests must continue to pass.
2. New GUI controller unit tests (Phases 3–5) — all new tests pass.
3. `integration_tests` — all existing integration tests pass.
4. `gui_integration_test` — updated to remove presenter dependencies.

After Phase 8:

5. Full `ctest` run with zero failures.
6. Manual smoke test on Linux target.

### 4.6 Performance Validation

| Metric                     | Target                | Measurement Method                                  |
| -------------------------- | --------------------- | --------------------------------------------------- |
| Node drop → canvas redraw  | ≤ 100ms               | `QElapsedTimer` in `TrussCanvasWidget::refresh()`   |
| Validation on edit (debounced) | ≤ 50ms            | Profile `validateTruss` call in `MainWindowController` |
| Analysis initiation → `analysisStarted` signal | ≤ 50ms | `QSignalSpy` timing in controller test |
| `NodeTableModel::refresh(100-node truss)` | ≤ 10ms | GTest benchmark |

---

## 5. Integration Strategy

### 5.1 Facade Dependency Rule

**One rule, no exceptions:** Every controller that needs to mutate or query data
receives the interface type. No controller includes a `*_impl.hpp` or concrete
class header from `src/interface/`, `src/application/`, or `src/core/`.

Enforced via include-what-you-use: a clang-tidy check (`-modernize-include-what-you-use`)
flags any GUI source file that transitively includes a concrete domain class.

### 5.2 Sub-Interface Distribution

`ITrussAnalysisFacade` inherits from both `ITrussService` and `IAnalysisService`. Controllers
receive the narrowest interface they need:

| Controller             | Receives                               | Why                                          |
| ---------------------- | -------------------------------------- | -------------------------------------------- |
| `CanvasController`     | `ITrussService*`                       | Only needs node/member mutation              |
| `InspectorController`  | `ITrussService*`                       | Only needs node/member mutation + view       |
| `AnalysisController`   | `IAnalysisService*`, `ITrussService*`  | Needs analysis + truss snapshot              |
| `ProjectController`    | `ITrussService*`                       | Only needs lifecycle + save/load             |
| `ExportController`     | `IAnalysisService*`                    | Only needs export                            |
| `MainWindowController` | `ITrussAnalysisFacade*`                | Orchestrates, needs full interface           |

`MainWindow` performs the sub-interface extraction:

```cpp
// In MainWindow constructor — only place sub-interface casts occur
m_canvasController = std::make_unique<CanvasController>(
    static_cast<ITrussService*>(&facade), this);
m_analysisController = std::make_unique<AnalysisController>(
    static_cast<IAnalysisService*>(&facade),
    static_cast<ITrussService*>(&facade), this);
```

### 5.3 Strict Separation — Architectural Enforcement Mechanisms

**1. Directory-level enforcement:** All Qt includes (`#include <Qt>`) are confined to
`src/gui/`. CI (if configured) can verify this with:
```bash
grep -r "#include <Qt" src/{core,application,interface,infrastructure,utilities}/
```
Expected: zero matches.

**2. CMake target boundary:** No domain/infrastructure CMake target gains a dependency
on `Qt6::Widgets`. The `TrussAnalysisGUI` target is the only target that links
`Qt6::Widgets`.

**3. Signal payload types:** Signals and slots use only primitive types, `QString`,
or types defined in `application/result.hpp` and `core/model/types.hpp` (handle types,
`NodeId`, `MemberId`, `Point2D`, `Force2D`). They never carry full domain objects
(`Truss&`, `Member&`, `Node&`).

**4. Read-only view discipline:** All domain-to-UI data flows through `ITrussView::getNodeViews()`
and `ITrussView::getMemberViews()` which return `std::vector<NodeView>` by value. No
`const Truss&` reference is held by any UI class beyond the scope of a single
`refresh()` call.

### 5.4 Dependency Injection

Construction order in `MainWindow` constructor (the composition root for the GUI
subsystem):

```
ITrussAnalysisFacade& facade
    → WorkspaceState (value, held by MainWindowController)
    → MainWindowController(facade)
    → CanvasController(ITrussService*)
    → InspectorController(ITrussService*)
    → AnalysisController(IAnalysisService*, ITrussService*)
    → ProjectController(ITrussService*)
    → ExportController(IAnalysisService*)
    → NodeTableModel, MemberTableModel, ValidationListModel
    → TrussCanvasWidget → set models
    → InspectorPanel(ValidationListModel*)
    → AnalysisControlBar
    → ResultsDockPanel(NodeTableModel*, MemberTableModel*)
    → NotificationRail
    → Connect all signals/slots
```

No `new` call outside of this construction sequence creates a controller or model.
All objects are either `std::unique_ptr` members of `MainWindow` or `QObject`-parented
children (which are deleted by Qt's parent chain).

---

## 6. Risk Analysis & Mitigation Plan

### 6.1 Technical Risks

| Risk | Probability | Impact | Mitigation |
| ---- | ----------- | ------ | ---------- |
| `AnalysisWorker` thread-safety: `IAnalysisService::analyze()` is not thread-safe (internal state in facade impl) | Medium | High | Pass a **value copy** of `core::Truss` to the worker; worker calls `analyze(const Truss&, opts)` with no shared state; the result handle is stored in the facade's thread-safe map |
| `QTransform` accumulation drift in canvas after many pan/zoom operations | Low | Medium | Reconstruct `m_worldToScreen` from `m_scale` and `m_panOffset` primitives on every paint, not via incremental `QTransform::translate()`/`scale()` calls |
| `QSplitter` persisted state conflicts between screen sizes | Low | Low | Persist splitter sizes in `QSettings`; apply with a `QTimer::singleShot(0, ...)` after `show()` to ensure window is sized first |
| Y-axis flip in canvas causes incorrect force arrow direction display | Medium | Medium | Centralise the world-to-screen transform; write a unit test that verifies screen coordinates of a known node position before any interaction logic is written |

### 6.2 Architectural Risks

| Risk | Probability | Impact | Mitigation |
| ---- | ----------- | ------ | ---------- |
| `MainWindowController` becomes a God object accumulating responsibilities | Medium | High | Strictly enforce: `MainWindowController` only owns `WorkspaceState` and orchestrates state transitions; it calls no facade methods itself (delegates to specific controllers) |
| Controller unit tests silently couple to Qt event loop via `QObject::connect` | Low | Medium | Default `Qt::DirectConnection` in tests (no event loop needed); never call `QCoreApplication::processEvents()` in unit tests |
| `ValidationListModel` click-to-select feature couples the model to controller logic | Low | Medium | Model emits `elementSelectionRequested(NodeId)` signal; `InspectorController` subscribes; model has no pointer to controller |

### 6.3 Scope Creep Risks

| Risk | Mitigation |
| ---- | ---------- |
| "While we're here" addition of undo/redo during canvas interaction implementation | Explicitly deferred. `WorkspaceState.isDirty` and project save cover the safety net. Log it as a post-v3.0.0 backlog item; do not implement. |
| Addition of a material/section property dialog beyond what `MaterialLibraryService` provides | `MaterialLibraryService` returns predefined materials. `InspectorPanel` exposes `QComboBox` populated from its list. Custom material entry is post-v3.0.0. |
| Animated canvas transitions for deformed shape | Not in scope. Static overlay is sufficient. |

### 6.4 Refactoring Regression Risks

| Risk | Mitigation |
| ---- | ---------- |
| Deleting old presenter classes removes the only objects that format `ValidationResult` for display; new `ValidationListModel` must replicate this logic correctly | Before deleting `ValidationPresenter`, extract its formatting logic into a `static` method in `ValidationListModel` and verify identical output in a comparison unit test |
| Old `AnalysisController` had a `getCurrentResults()` accessor used by integration tests | The new `AnalysisController` exposes `ResultsHandle currentResultsHandle() const`; update all call sites before deleting old controller |

---

## 7. Definition of Done

The migration is complete when **all** of the following criteria are met:

### 7.1 Architectural Compliance

- [ ] `src/gui/` contains only the files specified in architecture document Section 7.1.
- [ ] No file in `src/gui/` includes a header from `src/core/`, `src/application/`, or
      `src/infrastructure/` except the three interface ABI headers
      (`itruss_view.hpp`, `ianalysis_results_view.hpp`, DTOs).
- [ ] `cmake --build` with `BUILD_GUI=ON` links zero concrete domain class symbols into
      `TrussAnalysisGUI` beyond those reachable through `TrussInterface`.
- [ ] `grep -r "TrussApplicationService\|AnalysisApplicationService" src/gui/` → zero results.

### 7.2 Test Coverage

- [ ] All pre-existing unit tests (`unit_tests` binary) pass — zero regressions.
- [ ] All pre-existing integration tests (`integration_tests` binary) pass.
- [ ] New controller unit tests cover every public slot in each new controller.
- [ ] New model unit tests cover `rowCount`, `columnCount`, and `data()` for all
      `Qt::ItemDataRole` values implemented.
- [ ] `gui_integration_test` binary passes (updated test file, no dependency on deleted files).
- [ ] `ctest --test-dir build` reports 100% pass rate.

### 7.3 Code Quality

- [ ] `clang-format` applied to all new `src/gui/` files; no diff against format output.
- [ ] `clang-tidy` reports zero warnings on new `src/gui/` files using the project's
      existing `StaticAnalysis.cmake` configuration.
- [ ] No TODO or FIXME comments remain in any file in `src/gui/`.
- [ ] All new `Q_OBJECT` classes have corresponding Doxygen `@brief` documentation.

### 7.4 Functional Verification

- [ ] Manual smoke test on Linux: the following workflow completes without error:
  1. Launch `TrussAnalysisGUI`
  2. Open `examples/bridge-truss/*.json`
  3. Canvas renders the bridge truss geometry correctly
  4. Click Run Analysis → loading indicator appears → results panel populates
  5. Member Results tab shows stress ratio colour coding
  6. Export button → CSV → file written successfully
  7. Deformed shape overlay toggles on/off
  8. Theme toggle: switch dark ↔ light → persists after restart

### 7.5 Documentation

- [ ] `GUI-MODERNIZATION-ARCHITECTURE.md` header updated: Status → "Implemented: v3.0.0".
- [ ] `README.md` updated with new GUI feature list and keyboard shortcuts table.
- [ ] `CONTRIBUTING.md` updated to reflect the new `src/gui/` directory structure.

### 7.6 Legacy Artifact Removal

- [ ] `src/gui/widgets/interactive_drawing_widget.*` — deleted.
- [ ] `src/gui/widgets/node_input_widget.*`, `member_input_widget.*`,
      `load_input_widget.*`, `data_table_widget.*`, `plot_widget.*` — deleted.
- [ ] `src/gui/widgets/results_widget.*`, `deformed_truss_widget.*` — deleted.
- [ ] `src/gui/presenters/` directory — deleted entirely.
- [ ] `src/gui/controllers/analysis_controller.*`, `project_controller.*`,
      `truss_edit_controller.*` (old versions) — deleted.
- [ ] `tests/unit/gui/presenters/` — deleted.
- [ ] `tests/unit/gui/controllers/` old test files — replaced by new equivalents.
- [ ] `grep -r "InteractiveDrawingWidget\|TrussDataPresenter\|ValidationPresenter\|AnalysisResultsPresenter" src/ tests/` → zero results.

---

*End of Document*
