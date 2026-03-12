# GUI Layer Test Coverage Expansion

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 12, 2026  
**Status:** ✅ Complete  
**Branch:** `test/expand-coverage`  
**Scope:** Unit test coverage expansion across all GUI sub-layers: models, panels, widgets, and utilities

---

## 1. Executive Summary

This work log documents the systematic expansion of unit test coverage across the entire GUI layer. The base test suite had adequate coverage of domain, infrastructure, application, interface, and CLI layers but lacked targeted tests for Qt-specific display roles, widget interaction paths, panel lifecycle behaviours, and two GUI components (`ThemeLoader`, `AnalysisOptionsDialog`) that had no test coverage at all. This branch adds 1,473 lines of test code across 9 files — two new test suites and six extended ones — without modifying any production source.

---

## 2. Scope of Work

### New Test Suites

- Authored `tests/unit/gui/test_theme_loader.cpp`: 8 tests covering `ThemeLoader` static utility
- Authored `tests/unit/gui/panels/test_analysis_options_dialog.cpp`: 13 tests covering `AnalysisOptionsDialog` UI behaviour

### Extended Test Suites

- Extended `test_member_table_model.cpp`: `Qt::BackgroundRole` semantic colour tinting and `Qt::ToolTipRole` material property strings
- Extended `test_node_table_model.cpp`: `Qt::DisplayRole` `SupportType` enum formatting, post-analysis unit conversion display, and `Qt::ToolTipRole` behaviour
- Extended `test_results_table_model.cpp`: full `refresh()` / `clear()` lifecycle, unit conversions (metres → mm, Pa → MPa), `modelReset` signal emission semantics, and `Qt::TextAlignmentRole`
- Extended `test_inspector_panel.cpp`: `populateMaterialLibrary()` lazy initialisation path and dynamic repopulation while the editor is active
- Extended `test_results_dock_panel.cpp`: stiffness matrix lazy-loading across all three tab-navigation orderings
- Extended `test_truss_canvas_widget.cpp`: colour theme and display mode toggles, `zoomToFit()`, `RollerY` symbol rendering, `mouseMoveEvent`, `wheelEvent`, middle-button pan, `QEvent::PaletteChange`, and delete-mode click on an empty canvas

### Build System

- Registered `gui/test_theme_loader.cpp` and `gui/panels/test_analysis_options_dialog.cpp` in the `unit_tests_gui_widgets` CMake target

---

## 3. Technical Changes

### Qt Model Display Role Coverage

Three table model classes gained targeted `data()` coverage for Qt display roles that were previously untested:

- **`MemberTableModel`** — `Qt::BackgroundRole` returns a tinted `QBrush` for yielded (red), tension (blue), and compression (orange) states and returns `QVariant{}` before results are available or for non-state columns. `Qt::ToolTipRole` provides descriptive strings for the Area and Young's Modulus columns and the post-analysis state column.

- **`NodeTableModel`** — `Qt::DisplayRole` correctly formats `SupportType` domain enumerations (`RollerX`, `RollerY`, `Free`) into human-readable strings. Post-analysis displacement values are correctly converted from metres to millimetres and forces from Newtons to kilonewtons. `Qt::ToolTipRole` returns `QVariant{}` for non-support columns.

- **`ResultsTableModel`** — A `StubAnalysisResultsView` provides deterministic domain data for `refresh()`. Tests assert correct row count, key-value extraction, unit conversions (maximum displacement in mm, stress in MPa), `modelReset` emitted only when row data changes (not on idempotent `clear()` of an already-empty model), and left/right `Qt::TextAlignmentRole` alignment for key and value columns respectively.

### Panel Lifecycle Coverage

- **`InspectorPanel`** — `populateMaterialLibrary()` is exercised along both lifecycle paths: presets stored before the lazy-loaded `MemberEditorWidget` is first shown are applied on the initial `showEvent`; presets updated while the editor is already active trigger immediate `QComboBox` repopulation.

- **`ResultsDockPanel`** — A `StubResultsView` provides a known-size stiffness matrix. Three tab-navigation scenarios are covered: navigating to the stiffness-matrix tab after `setResultsView()` (populated), navigating before `setResultsView()` (empty), and calling `setResultsView()` while already on the tab (immediately populated).

### Widget Interaction Coverage

`TrussCanvasWidget` tests exercise the full interaction surface:

- `setColorTheme()` and `setDisplayMode()` are validated for correct state updates and idempotent early-return guards
- `zoomToFit()` adjusts the viewport to frame the model bounding box when nodes are present and resets to a default view when the canvas is empty
- `RollerY` support nodes produce a distinct render path verified to be crash-free
- `QMouseEvent` move with a loaded model emits `cursorPositionChanged`; `wheelEvent` zooms in and out; a synthesised middle-button press/move/release sequence pans the viewport without crash
- `QEvent::PaletteChange` is dispatched through `changeEvent()` without triggering assertions or crashes
- A left-click in delete mode on an empty (node-free) canvas completes without error

### New Component Coverage

- **`ThemeLoader`** — Validates the default theme reported as dark when no `QSettings` entry exists; graceful `false` return for invalid resource paths; successful QSS load for both built-in dark and light resources; round-trip persistence of theme selection to and from `QSettings`.

- **`AnalysisOptionsDialog`** — Validates default widget state against `AnalysisOptions` defaults; solver-type toggle that enables/disables tolerance and max-iterations spinboxes; bidirectional round-trip via `setOptions()` / `options()`; and convergence tolerance precision within `QDoubleSpinBox` limits.

---

## 4. Testing and Validation

- All 6 CTest targets pass: `cleanup_gcda`, `gtest_integration`, `unit_tests`, `unit_tests_gui_widgets`, `integration_tests`, `gui_integration_tests`
- Final assertion counts: 1,391 (`unit_tests`) + 142 (`unit_tests_gui_widgets`) = **1,533 total**
- `ThemeLoader` and `AnalysisOptionsDialog` test sources are correctly registered in the `unit_tests_gui_widgets` CMake target and guarded by `BUILD_GUI=ON`
- No production source files were modified

---

## 5. Outcome

The GUI layer now has comprehensive unit test coverage across all sub-layers: Qt data model display roles, widget user-interaction paths, panel lifecycle behaviours, and the two previously untested utility and dialog components. Stub implementations (`StubAnalysisResultsView`, `StubResultsView`) provide deterministic domain data for model-level tests without depending on live analysis execution. No public API, library interfaces, or cross-target dependency edges were modified. No breaking changes were introduced.
