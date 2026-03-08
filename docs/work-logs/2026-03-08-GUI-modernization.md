# GUI Modernization: Qt-Based Workbench Implementation

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 8, 2026  
**Status:** ✅ Complete  
**Scope:** Ground-up replacement of the legacy Qt GUI with a workspace-centric, MVC-based structural analysis workbench; full integration with the interface layer; decommissioning of all legacy view components

---

## Context

The legacy GUI coupled domain types directly to widget classes, lacked a controller layer, and provided no mechanism for unit testing view interactions. This work implements the architecture documented in `docs/architecture/GUI-MODERNIZATION-ARCHITECTURE.md`: a layered Qt Widgets application in which all domain interaction travels through `ITrussAnalysisFacade`, all state transitions are orchestrated by a `MainWindowController` hierarchy, and all view components bind to Qt item models that are populated from read-only domain views. The legacy implementation was progressively replaced and then fully decommissioned upon stabilization of the v2 system.

---

## Scope of Work

**Infrastructure**

- Implemented dark and light QSS themes using standard design tokens; registered in the Qt resource system
- Implemented `ThemeLoader` for QSS resource loading and runtime theme switching with OS palette synchronization
- Implemented `NotificationRail` for non-blocking, timed feedback display
- Added SVG icon resources for all toolbar actions, display mode toggles, canvas tools, and dialog controls

**Qt Item Models**

- Implemented `NodeTableModel`, `MemberTableModel`, `ValidationListModel`, and `ResultsTableModel` as `QAbstractTableModel` subclasses populated from `ITrussView` and `IAnalysisResultsView`

**Canvas Widget**

- Implemented `TrussCanvasWidget` 2D rendering pipeline with coordinate transformation, grid display, node and member rendering, support and load visualization, and deformed shape overlay
- Introduced `ToolMode` enum for canvas interaction state; implemented `setMode` slot and mode-specific cursor handling
- Implemented spatial hit-testing for node and member selection
- Implemented canvas panning, cursor-centric zoom via mouse wheel, and cursor coordinate tracking
- Implemented grid snapping for node placement
- Implemented reaction force vector rendering with correct orientation
- Added `nodeDropRequested`, `memberDrawRequested`, `nodeSelected`, `memberSelected`, `nodeDeleteRequested`, `memberDeleteRequested`, and `cursorPositionChanged` interaction signals
- Implemented deformed shape visualization with auto-scaled deformation magnification, clamped to standard visual limits
- Implemented display mode switching (`Geometry` / `DeformedShape`) with theme-aware member color coding by mechanical state
- Implemented zoom-to-fit for project load and creation events

**Panel Components**

- Implemented `InspectorPanel` for context-sensitive node and member property editing, including material library population, cross-section input, coordinate editing, support type selection, and load input; deferred creation of interactive member editor widgets until first selection
- Implemented `AnalysisControlBar` for analysis lifecycle controls (run, stop, options)
- Implemented `ResultsDockPanel` with tabbed views: analysis summary table, member results table with mechanical state color coding, and lazily populated stiffness matrix `QTableView`
- Implemented `AnalysisOptionsDialog` for solver configuration

**Controllers**

- Implemented `CanvasController`, `InspectorController`, `AnalysisController`, `ProjectController`, and `ExportController`; all depend exclusively on `ITrussService` and `IAnalysisService` interfaces
- Implemented `MainWindowController` as the GUI composition root: instantiates sub-controllers and Qt item models, wires inter-controller signals, and orchestrates workspace state transitions including model refresh cascades on truss modification and analysis completion
- Implemented a global Qt event filter in `MainWindow` for reliable keyboard shortcut handling and canvas focus restoration on item selection

**Main Window**

- Implemented `MainWindow` (v2) with full workspace layout: canvas area, inspector dock, results dock, toolbar strip, and status bar with node/member count labels and cursor coordinate display
- Wired all menu actions, toolbar actions, file dialogs, and tool mode toggles to domain controllers; unified `QAction` instances across menus and toolbars to prevent state divergence
- Implemented dirty-state guard on window close with unsaved-changes prompt
- Implemented theme persistence: last-used theme is restored on application startup
- Configured explicit keyboard tab navigation order in dialog panels

**Application Layer Extensions**

- Added `updateNode` and `updateMember` DTOs to `truss_edit_dtos.hpp`
- Added `updateNode` and `updateMember` to `ITrussService`, `TrussApplicationService`, `TrussAnalysisFacade`, and `ITrussAnalysisFacade`
- Extended `FacadeTrussServiceAdapter` to delegate the new update operations

**Legacy Decommissioning**

- Removed all legacy controller, presenter, and widget implementations and their associated test suites
- Migrated the GUI integration test suite to the v2 component set
- Removed legacy mock compatibility validation tests from the core test suite

**Build**

- Registered all new panel, controller, canvas, model, and infrastructure source files in the CMake `TrussAnalysisGUI` target
- Registered all new unit test source files in the `unit_tests` CMake target

**Documentation and Style**

- Standardised Doxygen comments across core and application interface headers
- Applied clang-format to all modified source, header, and test files

---

## Technical Changes

### MVC Structure

`MainWindowController` owns the sub-controller instances and Qt item models. It receives a reference to `ITrussAnalysisFacade` at construction and distributes `ITrussService` and `IAnalysisService` references to sub-controllers. No widget or panel class holds a direct service reference; all mutations are requested through controller slots connected to panel signals.

### Qt Item Model Binding

`NodeTableModel`, `MemberTableModel`, and `ResultsTableModel` store a pointer to the relevant domain view interface and implement `rowCount`, `columnCount`, and `data` without caching domain state. Model refresh is triggered explicitly by `MainWindowController` after any mutation or analysis completion, ensuring view consistency without polling.

### Canvas Rendering Pipeline

`TrussCanvasWidget` maintains a world-to-screen affine transformation updated on resize, pan, and zoom events. Node and member positions are stored as world coordinates; all rendering converts to screen coordinates at paint time. Deformed shape rendering applies a scale factor computed from the ratio of a fixed visual displacement magnitude to the maximum displacement in the results view, clamped to a configurable range.

### In-Place Entity Updates

`updateNode` and `updateMember` were added to `ITrussService` as non-destructive update operations driven by `NodeUpdateDto` and `MemberUpdateDto`. These complement the existing `removeNode`/`addNode` pattern used previously for coordinate changes, avoiding handle invalidation for inspector-driven edits. The operations propagate through the full stack: application service, facade, and adapter.

### Reaction Force Orientation

The canvas rendered reaction force vectors in the direction of the applied load rather than the reaction direction. The rendering logic was corrected to invert the vector direction for reaction force display, and a targeted unit test was added to assert the corrected sign convention.

### Keyboard Shortcut Reliability

A `QObject`-based event filter installed on the `QApplication` instance intercepts key events application-wide before Qt's focus-based routing. This resolves shortcut failures caused by focus leaving the canvas widget. The filter is installed on construction and removed on `MainWindow` destruction to avoid dangling filter registration.

---

## Testing and Validation

- Added unit tests for all new panel components (`InspectorPanel`, `AnalysisControlBar`, `ResultsDockPanel`).
- Added unit tests for all new controllers (`CanvasController`, `InspectorController`, `AnalysisController`, `ProjectController`, `ExportController`, `MainWindowController`).
- Added unit tests for all Qt item models (`NodeTableModel`, `MemberTableModel`, `ValidationListModel`, `ResultsTableModel`).
- Added unit tests for `TrussCanvasWidget` covering tool mode transitions, screen-to-world coordinate mapping, and interaction signal emissions.
- Added unit tests for `NotificationRail` and `WorkspaceState`.
- Added unit tests for `updateNode` and `updateMember` at both the application service and inspector controller layers.
- Added unit tests for canvas deformation scale clamping and reaction force vector orientation.
- Extended `MockTrussAnalysisFacade` with `updateNode` and `updateMember` mock methods.
- Removed all legacy GUI mock compatibility and data presenter test suites; no suppressed or skipped tests remain from those suites.
- Build verified stable with all targets correctly linked and no new compiler errors or warnings.

---

## Outcome

The GUI layer has been fully replaced with a Qt Widgets workbench that implements the architecture specified in `GUI-MODERNIZATION-ARCHITECTURE.md`. All domain interaction is mediated through `ITrussAnalysisFacade`; no widget or controller class holds a compile-time dependency on a concrete service or domain type beyond those exposed through the read-only view interfaces. The legacy view implementation has been completely removed. The `ITrussService` interface and its full implementation stack have been extended with non-destructive in-place entity update operations. No breaking changes were introduced to any public interface outside the GUI layer.
