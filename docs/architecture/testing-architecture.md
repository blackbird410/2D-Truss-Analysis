# Testing Architecture

## Overview

The test suite lives in `tests/` and is organized to mirror the source tree. Tests exercise every layer independently using Google Test and GoogleMock. All five CTest targets pass at 100% with 1,603 tests as of v3.0.0.

---

## Structural Diagram

```
┌───────────────────────────────────────────────────────────────┐
│                      TEST SUITE                               │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  unit/                                                  │  │
│  │  ├── core/          model, analysis, assembly, valid.   │  │
│  │  ├── application/   services, material library          │  │
│  │  ├── interface/     facade, adapters, builder           │  │
│  │  ├── infrastructure/exporters, I/O, logging, adapters   │  │
│  │  ├── cli/           commands, argument parser, presenter│  │
│  │  ├── gui/           controllers, models, panels, state  │  │
│  │  └── utilities/     math_utils, string_utils            │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  integration/   end-to-end workflow tests               │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  mocks/         MockTrussAnalysisFacade (GMock)         │  │
│  │                 MockTrussApplicationService             │  │
│  │                 MockAnalysisApplicationService          │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │  fixtures/      golden master files                     │  │
│  │  (CSV, JSON, XML, HTML, LaTeX, TXT)                     │  │
│  └─────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────────┘
```

---

## Test Targets (CTest)

| CTest Target             | Binary                    | Contains                                                                     |
| ------------------------ | ------------------------- | ---------------------------------------------------------------------------- |
| `unit_tests`             | `TrussUnitTests`          | Core, application, interface, infrastructure, CLI, utilities, non-widget GUI |
| `unit_tests_gui_widgets` | `TrussUnitTestsGui`       | Qt widget tests requiring `QApplication`                                     |
| `integration_tests`      | `TrussIntegrationTests`   | End-to-end workflow tests across all layers                                  |
| `gui_integration_test`   | `TrussGuiIntegrationTest` | GUI integration (controller + facade + domain)                               |
| `test_gtest_integration` | `TrussGTestIntegration`   | Basic GTest framework self-test                                              |

All targets run in CI. A custom `test_main.cpp` ensures `QCoreApplication` is constructed on the stack so Qt cleanup completes before gcov atexit handlers run.

---

## Unit Test Coverage by Layer

### Core — `tests/unit/core/`

| Sub-directory | Test Files                                                                                                                                                                                             |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `model/`      | `test_node.cpp`, `test_member.cpp`, `test_member_branches.cpp`, `test_load.cpp`, `test_truss.cpp`, `test_truss_branches.cpp`, `test_stiffness_matrix_bug.cpp`                                          |
| `analysis/`   | `test_analysis_orchestrator.cpp`, `test_stiffness_assembler.cpp`, `test_boundary_condition_handler.cpp`, `test_linear_solvers.cpp`, `test_displacement_sign_check.cpp`, `test_warren_displacement.cpp` |
| `assembly/`   | `test_truss_assembler.cpp` (DTO ↔ domain round-trips)                                                                                                                                                  |
| `validation/` | `test_truss_validator.cpp`, `test_truss_validator_branches.cpp`, `test_validation_advanced.cpp`                                                                                                        |

### Application — `tests/unit/application/`

| Test File                                        | Covers                                              |
| ------------------------------------------------ | --------------------------------------------------- |
| `test_truss_application_service.cpp`             | Lifecycle, CRUD, handle management                  |
| `test_truss_application_service_gui_methods.cpp` | Edit operations (addNode, addMember, setSupport, …) |
| `test_truss_application_service_update.cpp`      | updateNode, updateMember                            |
| `test_analysis_application_service.cpp`          | analyze(), getResultsView(), exportResults()        |
| `test_material_library_service.cpp`              | Preset queries, custom registration                 |

### Interface — `tests/unit/interface/`

| Test File                                  | Covers                                                 |
| ------------------------------------------ | ------------------------------------------------------ |
| `test_truss_analysis_facade.cpp`           | Full facade workflows (analyzeFromFile, etc.)          |
| `test_facade_truss_service_adapter.cpp`    | Adapter delegation — verifies no facade logic bypassed |
| `test_facade_analysis_service_adapter.cpp` | Adapter delegation for analysis operations             |
| `test_truss_builder.cpp`                   | Builder fluent API, referential integrity validation   |

### Infrastructure — `tests/unit/infrastructure/`

| Sub-directory | Test Files                                                                   |
| ------------- | ---------------------------------------------------------------------------- |
| `export/`     | Per-format exporter tests: CSV, JSON, XML, HTML, LaTeX, TXT (golden masters) |
| `io/`         | JSON reader/writer, XML reader/writer, round-trip serialization              |
| `logging/`    | ConsoleLogger, FileLogger level filtering                                    |
| `adapters/`   | ConsoleOutputAdapter                                                         |

### CLI — `tests/unit/cli/`

| Test File                        | Covers                                                       |
| -------------------------------- | ------------------------------------------------------------ |
| `test_analyze_command.cpp`       | `AnalyzeCommand::execute()` — uses `MockTrussAnalysisFacade` |
| `test_validate_command.cpp`      | `ValidateCommand::execute()`                                 |
| `test_validate_command_mock.cpp` | Mock-based isolation of ValidateCommand                      |
| `test_export_command.cpp`        | `ExportCommand::execute()`                                   |
| `test_export_command_mock.cpp`   | Mock-based isolation                                         |
| `test_example_command.cpp`       | `ExampleCommand::execute()`                                  |
| `test_help_command.cpp`          | `HelpCommand::execute()`                                     |
| `test_argument_parser.cpp`       | Argument parsing, flag extraction                            |
| `test_console_presenter.cpp`     | Output formatting                                            |

### GUI — `tests/unit/gui/`

| Sub-directory  | Test Files                                                                                                                                                                                    |
| -------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `controllers/` | `test_main_window_controller.cpp`, `test_canvas_controller.cpp`, `test_inspector_controller.cpp`, `test_analysis_controller.cpp`, `test_project_controller.cpp`, `test_export_controller.cpp` |
| `models/`      | `test_node_table_model.cpp`, `test_member_table_model.cpp`, `test_results_table_model.cpp`, `test_validation_list_model.cpp`                                                                  |
| `state/`       | `test_workspace_state.cpp` — WorkspacePhase transitions                                                                                                                                       |
| `panels/`      | Panel-specific tests                                                                                                                                                                          |
| `widgets/`     | Widget-specific tests                                                                                                                                                                         |
| (root)         | `test_theme_loader.cpp`                                                                                                                                                                       |

All controller tests inject `MockTrussAnalysisFacade` and verify correct delegation.

---

## Integration Tests — `tests/integration/`

End-to-end tests exercising the full stack from CLI command or facade call through to domain model, analysis, and export output.

| Test File                              | Covers                                                |
| -------------------------------------- | ----------------------------------------------------- |
| `test_e2e_workflows.cpp`               | Complete load → validate → analyze → export pipelines |
| `test_simple_integration.cpp`          | Simple truss analysis correctness                     |
| `test_simple_truss_analysis.cpp`       | Known-result verification (numerical regression)      |
| `test_minimal_analysis.cpp`            | Minimal valid truss                                   |
| `test_load_workflow.cpp`               | File load workflows (JSON + XML)                      |
| `test_lifecycle_integration.cpp`       | Handle lifecycle — create, analyze, clear, re-create  |
| `test_facade_adapters_integration.cpp` | Adapter round-trips through the full facade           |
| `test_cli_workflows.cpp`               | CLI command dispatch over real facade                 |
| `test_working_integration.cpp`         | Cross-layer smoke test                                |
| `test_gui_integration.cpp`             | GUI controller + facade + domain round-trip           |

---

## Mocking Strategy

All mock objects live in `tests/mocks/`.

| Mock                             | Implements             | Used By                                                |
| -------------------------------- | ---------------------- | ------------------------------------------------------ |
| `MockTrussAnalysisFacade`        | `ITrussAnalysisFacade` | CLI command tests, GUI controller tests, adapter tests |
| `MockTrussApplicationService`    | `ITrussService`        | Application-layer isolation tests                      |
| `MockAnalysisApplicationService` | `IAnalysisService`     | Application-layer isolation tests                      |

All mocks are generated with GoogleMock `MOCK_METHOD` macros. Tests use `EXPECT_CALL` to verify correct delegation without invoking the real domain or infrastructure logic.

---

## Golden Masters — `tests/fixtures/`

The `fixtures/` directory contains reference output files for every supported export format. Infrastructure exporter tests compare actual output byte-for-byte (or line-by-line) against these golden masters. Deterministic build metadata is used to avoid timestamp-related mismatches.

Supported golden master formats: CSV, JSON, XML, HTML, LaTeX, TXT.

---

## Coverage Metrics (v3.0.0)

| Metric            | Value |
| ----------------- | ----- |
| Total tests       | 1,603 |
| Pass rate         | 100%  |
| Line coverage     | 88.3% |
| Function coverage | 91.9% |
| Branch coverage   | 51.8% |

Coverage is generated via `lcov` / `genhtml` using the `scripts/generate_coverage.sh` script. Reports are published to `docs/`.

---

## Key Design Constraints

- Every layer is tested in isolation: application tests never instantiate Qt; GUI tests never hit the real file system.
- Controller and CLI command tests receive `MockTrussAnalysisFacade` via constructor injection — no static globals or test-only singletons.
- Integration tests use the real `TrussAnalysisFacade` with the real file system; they are always run in a temporary directory.
- GUI tests that require `QCoreApplication` (item models, controllers) share the application object via a file-local `ensureQApp()` helper.
- Tests that require `QApplication` (widget rendering) are isolated in the separate `TrussUnitTestsGui` binary to prevent Qt platform plugin issues in headless CI.
- Golden master files are committed alongside tests. Any exporter change that alters output format must update the corresponding fixture before the test suite passes.
