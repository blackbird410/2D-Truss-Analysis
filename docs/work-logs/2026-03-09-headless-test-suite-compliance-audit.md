# Headless Test Suite Compliance Audit

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 9, 2026  
**Status:** ✅ Complete  
**Branch:** `ci/implement-headless-test-suite`  
**Base:** `build/application-packaging-and-distribution-strategy`  
**Scope:** Qt platform plugin failures in CI, test binary organisation, CTest environment configuration, CI workflow correctness

---

## 1. Headless Compatibility Assessment

### Symptom

Tests in the CI `Run unit tests` step failed in the GitHub Actions environment with output similar to:

```
qt.qpa.xcb: could not connect to display
QXcbConnection: Could not connect to display
Aborted (core dumped)
```

### Root Causes

Four separate defects combined to produce the failures.

#### RC-1 — Widget tests compiled into `unit_tests` without `QT_QPA_PLATFORM=offscreen`

The `unit_tests` binary included panel widget tests (`NotificationRail`, `AnalysisControlBar`, `InspectorPanel`, `ResultsDockPanel`) and the `TrussCanvasWidget` test alongside purely logical Qt tests. All of these panel and widget tests create `QWidget` subclass instances during their `SetUp()` phase, which causes Qt to attempt to load a Qt platform plugin. In the GitHub Actions `ubuntu-latest` environment no X11 server is present, so Qt's default selection of the `xcb` plugin fails immediately.

The `unit_tests` CTest target had no `ENVIRONMENT` property set, and the corresponding CI step had no `QT_QPA_PLATFORM: offscreen` in its `env:` block. Qt therefore followed its normal plugin discovery path (`xcb` first), which crashed before any test executed.

#### RC-2 — `QApplication` initialised by non-display tests unnecessarily

The following test files used `QApplication` in their bootstrap even though the code under test contains no widget interaction at runtime:

| Test file                         | Class under test       | Actual Qt dependency                       |
| --------------------------------- | ---------------------- | ------------------------------------------ |
| `test_node_table_model.cpp`       | `NodeTableModel`       | `QAbstractTableModel` (QtCore)             |
| `test_member_table_model.cpp`     | `MemberTableModel`     | `QAbstractTableModel` (QtCore)             |
| `test_results_table_model.cpp`    | `ResultsTableModel`    | `QAbstractTableModel` (QtCore)             |
| `test_canvas_controller.cpp`      | `CanvasController`     | `QObject` + `QSignalSpy`                   |
| `test_analysis_controller.cpp`    | `AnalysisController`   | `QObject`, `QThread`                       |
| `test_inspector_controller.cpp`   | `InspectorController`  | `QObject` + `QSignalSpy`                   |
| `test_project_controller.cpp`     | `ProjectController`    | `QObject` + `QSignalSpy`                   |
| `test_export_controller.cpp`      | `ExportController`     | `QObject` + `QSignalSpy`                   |
| `test_main_window_controller.cpp` | `MainWindowController` | `QObject`, sub-controllers, Qt Item Models |

`QApplication` initialises the full Qt platform subsystem including display plugin loading. `QCoreApplication` does not. Initialising `QApplication` in these tests served no functional purpose and pulled the platform plugin loader into every test run that included these test objects in the binary.

#### RC-3 — `test_e2e_workflows.cpp` gated behind `BUILD_GUI`

`test_e2e_workflows.cpp` exclusively tests domain and infrastructure layer behaviour (assembly, analysis, export). It contains no Qt includes and carries no dependency on `Qt6::Widgets` or any display component. The file was placed inside a `if(BUILD_GUI)` guard in `INTEGRATION_TEST_SOURCES`, which excluded it from CLI-only and headless builds. This was a misclassification; the test should run unconditionally.

#### RC-4 — CTest regex patterns ambiguous, leading to double execution and incorrect test grouping

The CI `Run integration tests` step used `--tests-regex "integration_tests"`, which matched the test name `gui_integration_tests` (substring match) in addition to the intended `integration_tests`. This caused `gui_integration_tests` — which creates `QApplication` and instantiates `MainWindowController` — to run in a step that did not set `QT_QPA_PLATFORM`. While RC-1 already supplied the CTest `ENVIRONMENT` fix that would protect against this after correction, the regex ambiguity re-exposed the risk. Likewise, the `Run unit tests` step matched `unit_tests` and `unit_tests_gui_widgets` (the new binary) because the former is a prefix of the latter.

---

## 2. Tests Requiring UI

The following tests instantiate `QWidget` subclasses or use `QPixmap`/`QPainter` directly and therefore require a Qt platform plugin at runtime:

| Test file                                              | Reason                                                                                    |
| ------------------------------------------------------ | ----------------------------------------------------------------------------------------- |
| `tests/unit/gui/panels/test_notification_rail.cpp`     | Constructs `NotificationRail` (`QWidget`)                                                 |
| `tests/unit/gui/panels/test_analysis_control_bar.cpp`  | Constructs `AnalysisControlBar` (`QWidget`)                                               |
| `tests/unit/gui/panels/test_inspector_panel.cpp`       | Constructs `InspectorPanel` (`QStackedWidget`)                                            |
| `tests/unit/gui/panels/test_results_dock_panel.cpp`    | Constructs `ResultsDockPanel` (`QWidget`)                                                 |
| `tests/unit/gui/widgets/test_truss_canvas_widget.cpp`  | Constructs `TrussCanvasWidget` (`QGraphicsView`)                                          |
| `tests/unit/gui/models/test_validation_list_model.cpp` | Calls `Qt::DecorationRole` which constructs `QPixmap` via `QPainter`                      |
| `tests/integration/test_gui_integration.cpp`           | Creates `MainWindowController` which wires sub-controllers; uses `QSignalSpy` on GUI path |

All of the above function correctly with `QT_QPA_PLATFORM=offscreen`; none require an X11 server, Wayland compositor, or `xvfb`.

---

## 3. Refactoring Actions

### Action A — Split `unit_tests` into two binaries

A new CMake executable `unit_tests_gui_widgets` was introduced under the `if(BUILD_GUI AND BUILD_TESTING)` guard. It contains the six panel tests, the canvas widget test, and the `ValidationListModel` test (which invokes `QPixmap` via the decoration-role path), together with all required panel and model source files.

The `unit_tests` binary retains only tests whose subjects are `QObject` or `QAbstractItemModel` subclasses with no display interaction: the three remaining item model tests (`NodeTableModel`, `MemberTableModel`, `ResultsTableModel`), all six controller tests, and the `WorkspaceState` value-type test. `validation_list_model.cpp` is compiled into `unit_tests` as a source unit (without a test) because `MainWindowController` instantiates a `ValidationListModel` and therefore requires the symbol at link time; however, the decoration-role code path that calls `QPixmap` is never exercised by any controller test at runtime.

### Action B — `QApplication` replaced with `QCoreApplication` in non-display tests

The `ensureQApp()` bootstrap function in the following files was updated to construct and return `QCoreApplication` instead of `QApplication`:

- `tests/unit/gui/models/test_node_table_model.cpp`
- `tests/unit/gui/models/test_member_table_model.cpp`
- `tests/unit/gui/models/test_results_table_model.cpp`
- `tests/unit/gui/controllers/test_canvas_controller.cpp`
- `tests/unit/gui/controllers/test_analysis_controller.cpp`
- `tests/unit/gui/controllers/test_inspector_controller.cpp`
- `tests/unit/gui/controllers/test_project_controller.cpp`
- `tests/unit/gui/controllers/test_export_controller.cpp`
- `tests/unit/gui/controllers/test_main_window_controller.cpp`

The `#include <QApplication>` directive was removed from each file. Each bootstrap comment was updated to document the rationale. Panel and widget tests in `unit_tests_gui_widgets` retain `QApplication` and the existing `ensureQApp()` pattern unchanged, as they correctly require the full GUI subsystem.

### Action C — `ENVIRONMENT "QT_QPA_PLATFORM=offscreen"` added to CTest targets

`set_tests_properties` was updated for `unit_tests`, `unit_tests_gui_widgets`, and `gui_integration_tests` to include:

```cmake
ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
```

CTest propagates this property to the test process environment before launching the executable, ensuring the offscreen platform is selected regardless of the invoking environment. This makes the configuration self-contained: a `ctest` invocation in any CI or developer environment will supply the correct platform without requiring the caller to set the variable manually.

The `unit_tests` target receives the property as belt-and-suspenders: because it now initialises `QCoreApplication`, no platform plugin is loaded under normal operation, but the property guards against any future test introduced into the binary that inadvertently triggers `QGuiApplication` initialisation.

### Action D — `test_e2e_workflows.cpp` de-gated from `BUILD_GUI`

`test_e2e_workflows.cpp` was removed from the `if(BUILD_GUI)` conditional block in `INTEGRATION_TEST_SOURCES` and added unconditionally. This ensures it executes in all build configurations including CLI-only (`-DBUILD_GUI=OFF`) and all headless CI presets.

### Action E — CI workflow anchored test regex patterns

The `--tests-regex` arguments in `build-and-test.yml` were updated from unanchored substring matches to anchored regular expressions:

| Step                        | Old regex                | New regex                 |
| --------------------------- | ------------------------ | ------------------------- |
| Run unit tests              | `unit_tests`             | `^unit_tests$`            |
| Run integration tests       | `integration_tests`      | `^integration_tests$`     |
| Run GUI integration tests   | `gui_integration_test`   | `^gui_integration_tests$` |
| Run GTest integration tests | `test_gtest_integration` | `^gtest_integration$`     |

A new CI step `Run GUI widget unit tests (headless)` was added between `Run unit tests` and `Run integration tests`:

```yaml
- name: Run GUI widget unit tests (headless)
  env:
    QT_QPA_PLATFORM: offscreen
  run: |
    cd build
    ctest --output-on-failure \
          --tests-regex "^unit_tests_gui_widgets$" \
          --timeout 60 \
          --parallel $(nproc)
```

The `build/unit_tests_gui_widgets` executable was added to the `Verify executables` step. The `xvfb-run` wrapper was removed from the `Run GUI integration tests (headless)` step because the `offscreen` platform plugin does not require a virtual framebuffer.

---

## 4. CI Compatibility Status

The complete test suite was built and executed locally in a display-free environment to confirm headless compatibility:

| CTest target             | Initialises        | Platform                            | Result    |
| ------------------------ | ------------------ | ----------------------------------- | --------- |
| `gtest_integration`      | none               | n/a                                 | ✅ Passed |
| `unit_tests`             | `QCoreApplication` | none loaded                         | ✅ Passed |
| `unit_tests_gui_widgets` | `QApplication`     | `offscreen` (via CTest ENVIRONMENT) | ✅ Passed |
| `integration_tests`      | none               | n/a                                 | ✅ Passed |
| `gui_integration_tests`  | `QApplication`     | `offscreen` (via CTest ENVIRONMENT) | ✅ Passed |

All tests passed with a `ctest` invocation that sets no `QT_QPA_PLATFORM` in the calling environment. The platform property is supplied exclusively through the CTest `ENVIRONMENT` property, confirming that the configuration is portable to any CI environment.

The `unit_tests` binary was also verified to execute correctly without `QT_QPA_PLATFORM` in the environment at all, confirming that `QCoreApplication` avoids platform plugin loading entirely.

---

## 5. Final Test Architecture

```
tests/
├── unit/
│   ├── core/                 # Domain layer — no Qt
│   ├── infrastructure/       # Export, logging, I/O — no Qt
│   ├── application/          # App service layer — no Qt
│   ├── interface/            # Public facade — no Qt
│   ├── cli/                  # CLI commands — no Qt
│   ├── utilities/            # Math, string utils — no Qt
│   └── gui/
│       ├── state/            # WorkspaceState value type — no Qt
│       ├── models/           # QAbstractItemModel (QtCore) — QCoreApplication
│       │   ├── test_node_table_model.cpp
│       │   ├── test_member_table_model.cpp
│       │   ├── test_results_table_model.cpp
│       │   └── test_validation_list_model.cpp  → unit_tests_gui_widgets
│       ├── controllers/      # QObject controllers — QCoreApplication
│       │   └── (all 6 controller tests)
│       ├── panels/           # QWidget panels — QApplication + offscreen
│       │   └── (4 panel tests)             → unit_tests_gui_widgets
│       └── widgets/          # QGraphicsView widgets — QApplication + offscreen
│           └── test_truss_canvas_widget.cpp  → unit_tests_gui_widgets
├── integration/
│   ├── test_e2e_workflows.cpp         # Domain/infra — no Qt, always compiled
│   ├── test_gui_integration.cpp       # Signal/slot wiring — QApplication + offscreen
│   └── (other integration tests)     # QCoreApplication or no Qt
└── test_gtest_integration.cpp         # GTest smoke test — no Qt
```

### CMake test targets

| Target                   | Binary                   | Qt init            | CTest ENVIRONMENT            | CI step                     |
| ------------------------ | ------------------------ | ------------------ | ---------------------------- | --------------------------- |
| `gtest_integration`      | `test_gtest_integration` | none               | —                            | Run GTest integration tests |
| `unit_tests`             | `unit_tests`             | `QCoreApplication` | `QT_QPA_PLATFORM=offscreen`¹ | Run unit tests              |
| `unit_tests_gui_widgets` | `unit_tests_gui_widgets` | `QApplication`     | `QT_QPA_PLATFORM=offscreen`  | Run GUI widget unit tests   |
| `integration_tests`      | `integration_tests`      | none               | —                            | Run integration tests       |
| `gui_integration_tests`  | `gui_integration_test`   | `QApplication`     | `QT_QPA_PLATFORM=offscreen`  | Run GUI integration tests   |

¹ Belt-and-suspenders; no plugin is loaded because `QCoreApplication` is used. The property guards against future regressions.

### Layer segregation compliance

- **Domain layer tests** (`core/`, `infrastructure/`, `application/`) carry no Qt dependency whatsoever.
- **Controller tests** initialise `QCoreApplication`; sub-controller and item model subjects are `QObject`/`QAbstractItemModel` subclasses with no display interaction.
- **Widget and panel tests** are isolated in `unit_tests_gui_widgets` and require the `offscreen` platform plugin, which is satisfied without a physical display.
- **GUI integration tests** are isolated in the `gui_integration_tests` CTest target with `offscreen` set.

---

## Files Modified

| File                                                         | Change                                                                                                                                                                                                                   |
| ------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `CMakeLists.txt`                                             | Restructured `UNIT_TEST_SOURCES` BUILD_GUI block; added `unit_tests_gui_widgets` target; added CTest `ENVIRONMENT` to three targets; moved `test_e2e_workflows.cpp` out of `BUILD_GUI` guard; extended `all_gtest_tests` |
| `tests/unit/gui/models/test_node_table_model.cpp`            | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/models/test_member_table_model.cpp`          | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/models/test_results_table_model.cpp`         | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/controllers/test_canvas_controller.cpp`      | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/controllers/test_analysis_controller.cpp`    | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/controllers/test_inspector_controller.cpp`   | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/controllers/test_project_controller.cpp`     | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/controllers/test_export_controller.cpp`      | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `tests/unit/gui/controllers/test_main_window_controller.cpp` | `QApplication` → `QCoreApplication`                                                                                                                                                                                      |
| `.github/workflows/build-and-test.yml`                       | Added `unit_tests_gui_widgets` to verify step; added GUI widget test step; anchored all `--tests-regex` patterns; removed `xvfb-run`                                                                                     |
