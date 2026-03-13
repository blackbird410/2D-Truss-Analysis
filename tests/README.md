# Test Directory Structure

This directory contains all tests for the 2D Truss Analysis project, organized by layer and component using Google Test framework.

**Status:** ✅ 1603/1603 tests passing (100%)  
**Coverage:** 72%+ line coverage (estimated)  
**Execution Time:** ~950ms total

## Directory Structure

```
tests/
├── unit/                    # Unit tests (1533 passing across 2 executables)
│   ├── application/        # Application layer tests
│   │   ├── test_analysis_application_service.cpp
│   │   ├── test_material_library_service.cpp
│   │   ├── test_truss_application_service.cpp
│   │   ├── test_truss_application_service_gui_methods.cpp
│   │   └── test_truss_application_service_update.cpp
│   ├── cli/                # CLI layer tests
│   │   ├── test_analyze_command.cpp
│   │   ├── test_argument_parser.cpp
│   │   ├── test_console_presenter.cpp
│   │   ├── test_example_command.cpp
│   │   ├── test_export_command.cpp
│   │   ├── test_export_command_mock.cpp
│   │   ├── test_help_command.cpp
│   │   ├── test_validate_command.cpp
│   │   └── test_validate_command_mock.cpp
│   ├── core/               # Core domain tests
│   │   ├── analysis/
│   │   │   ├── test_analysis_orchestrator.cpp
│   │   │   ├── test_boundary_condition_handler.cpp
│   │   │   ├── test_displacement_sign_check.cpp
│   │   │   ├── test_linear_solvers.cpp
│   │   │   ├── test_stiffness_assembler.cpp
│   │   │   └── test_warren_displacement.cpp
│   │   ├── assembly/
│   │   │   └── test_truss_assembler.cpp
│   │   ├── model/
│   │   │   ├── test_load.cpp
│   │   │   ├── test_member.cpp
│   │   │   ├── test_member_branches.cpp
│   │   │   ├── test_node.cpp
│   │   │   ├── test_stiffness_matrix_bug.cpp
│   │   │   ├── test_truss.cpp
│   │   │   └── test_truss_branches.cpp
│   │   └── validation/
│   │       ├── test_truss_validator.cpp
│   │       ├── test_truss_validator_branches.cpp
│   │       └── test_validation_advanced.cpp
│   ├── gui/                # GUI layer tests (unit_tests_gui_widgets executable)
│   │   ├── controllers/
│   │   │   ├── test_analysis_controller.cpp
│   │   │   ├── test_canvas_controller.cpp
│   │   │   ├── test_export_controller.cpp
│   │   │   ├── test_inspector_controller.cpp
│   │   │   ├── test_main_window_controller.cpp
│   │   │   └── test_project_controller.cpp
│   │   ├── models/
│   │   │   ├── test_member_table_model.cpp
│   │   │   ├── test_node_table_model.cpp
│   │   │   ├── test_results_table_model.cpp
│   │   │   └── test_validation_list_model.cpp
│   │   ├── panels/
│   │   │   ├── test_analysis_control_bar.cpp
│   │   │   ├── test_analysis_options_dialog.cpp
│   │   │   ├── test_inspector_panel.cpp
│   │   │   ├── test_notification_rail.cpp
│   │   │   └── test_results_dock_panel.cpp
│   │   ├── state/
│   │   │   └── test_workspace_state.cpp
│   │   ├── widgets/
│   │   │   └── test_truss_canvas_widget.cpp
│   │   └── test_theme_loader.cpp
│   ├── infrastructure/     # Infrastructure layer tests
│   │   ├── adapters/
│   │   │   └── test_console_output_adapter.cpp
│   │   ├── export/
│   │   │   ├── test_csv_exporter.cpp
│   │   │   ├── test_export_advanced.cpp
│   │   │   ├── test_exporter_factory.cpp
│   │   │   ├── test_html_exporter.cpp
│   │   │   ├── test_json_exporter.cpp
│   │   │   ├── test_latex_exporter.cpp
│   │   │   ├── test_text_exporter.cpp
│   │   │   └── test_xml_exporter.cpp
│   │   ├── io/
│   │   │   ├── test_fileio.cpp
│   │   │   ├── test_fileio_advanced.cpp
│   │   │   ├── test_json_reader_branches.cpp
│   │   │   ├── test_support_type_serializer.cpp
│   │   │   └── test_xml_reader_branches.cpp
│   │   └── logging/
│   │       ├── test_logger.cpp
│   │       └── test_logger_advanced.cpp
│   ├── interface/          # Interface / facade tests
│   │   ├── test_facade_analysis_service_adapter.cpp
│   │   ├── test_facade_truss_service_adapter.cpp
│   │   ├── test_truss_analysis_facade.cpp
│   │   └── test_truss_builder.cpp
│   ├── utilities/          # Utility tests
│   │   ├── test_math_utils.cpp
│   │   ├── test_string_utils.cpp
│   │   └── test_string_utils_branches.cpp
│   ├── test_main.cpp       # unit_tests entry point
│   └── test_main_gui.cpp   # unit_tests_gui_widgets entry point
│
├── integration/             # Integration tests (67 passing across 2 executables)
│   ├── test_cli_workflows.cpp
│   ├── test_e2e_workflows.cpp
│   ├── test_facade_adapters_integration.cpp
│   ├── test_gui_integration.cpp
│   ├── test_lifecycle_integration.cpp
│   ├── test_load_workflow.cpp
│   ├── test_minimal_analysis.cpp
│   ├── test_simple_integration.cpp
│   ├── test_simple_truss_analysis.cpp
│   └── test_working_integration.cpp
│
├── fixtures/                # Shared test data and helpers
│   ├── export_golden/      # Golden master files for export validation
│   └── test_data/          # JSON/XML test truss structures
│
└── test_gtest_integration.cpp  # GTest framework validation (3 tests)
```

## Test Statistics

### Overall Metrics

- **Total Tests:** 1603
- **Passing:** 1603 (100%)
- **Skipped:** 0
- **Failed:** 0
- **Execution Time:** ~950ms total
  - `unit_tests` (headless): 1391 tests, 224ms
  - `unit_tests_gui_widgets`: 142 tests, 695ms
  - `integration_tests`: 54 tests, 9ms
  - `gui_integration_test`: 13 tests, 19ms
  - `test_gtest_integration`: 3 tests, <1ms

### Phase 8 Additions (February 2026)

- GUI Layer: +8 tests (rapid operations, error recovery)
- Export Layer: +8 tests (edge cases, large files)
- FileIO Layer: +7 tests (concurrent I/O, Unicode)
- Validation Layer: +19 tests (extreme geometries)
- Logger Layer: +22 tests (concurrent logging, stress)
- Integration E2E: +7 tests (complete workflows)

## Test Organization Principles

### Unit Tests (`tests/unit/`)

- **Framework:** Google Test (GTest) 1.17.0 with GMock for mocking
- **Purpose:** Test individual classes or functions in isolation
- **CLI Testing Strategy:** Simplified mock-based approach with minimal output interfaces
- **Characteristics:**
  - Layer-isolated tests with comprehensive mocking
  - Fast execution (< 1ms per test typical)
  - Professional documentation standards
  - 1391/1391 tests passing (100% pass rate)
- **Target:** `unit_tests` (headless) and `unit_tests_gui_widgets` (Qt display required)

### Integration Tests (`tests/integration/`)

- **Purpose:** Test multiple components working together
- **Characteristics:**
  - Real implementations (minimal mocking)
  - End-to-end CLI workflow validation
  - Moderate execution time (< 100ms per test typical)
  - Workflow-level validation
- **Target:** `integration_tests` (aggregate executable)

### Test Fixtures (`tests/fixtures/`)

- **Purpose:** Shared test data, helper functions, and reusable test infrastructure
- **Characteristics:**
  - Header-only utilities (no separate compilation)
  - Common test data generators
  - Assertion helpers for numerical comparisons

## Test Target Naming

### CMake Targets

- `test_gtest_integration` - GTest framework validation (3 tests)
- `unit_tests` - Core + CLI + infrastructure + application + interface + utilities tests
- `unit_tests_gui_widgets` - GUI layer tests (requires Qt display)
- `integration_tests` - Integration tests (54 tests)
- `gui_integration_tests` - GUI integration tests (13 tests)

### CTest Labels

- `unit` - Unit test label (filter with `ctest -L unit`)
- `integration` - Integration test label (filter with `ctest -L integration`)
- `gtest` - GTest framework label

## Migration Status

**Current State (Phase 1 Step 1.3):**

- ✅ New directory structure created
- ✅ Directory documented
- ⏳ Tests not yet migrated (remains in `tests/unit/`)
- ⏳ Legacy TestFramework.hpp still present

**Next Step (Phase 1 Step 1.4):**

- Migrate production tests to new structure
- Create test fixtures as needed
- Remove legacy tests after migration

## Test Execution

### Run All Tests

```bash
ctest --output-on-failure
```

### Run Unit Tests Only

```bash
ctest -L unit --output-on-failure
```

### Run Integration Tests Only

```bash
ctest -L integration --output-on-failure
```

### Run Specific Test

```bash
ctest -R test_node --output-on-failure
```

### Run Tests with Verbose Output

```bash
ctest --verbose
```

## References

- [Phase 1 Step 1.1 - Test Framework Audit](../docs/work-logs/2026-02-05-phase-1-step-1-test-audit.md)
- [Phase 1 Step 1.2 - GTest Integration](../docs/work-logs/2026-02-05-phase-1-step-2-gtest-integration.md)
