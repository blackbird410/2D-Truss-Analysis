# Test Directory Structure

This directory contains all tests for the 2D Truss Analysis project, organized by layer and component using Google Test framework.

**Status:** ✅ 765/766 tests passing (99.87%)  
**Coverage:** 72%+ line coverage (estimated)  
**Execution Time:** 148ms total

## Directory Structure

```
tests/
├── unit/                    # Unit tests (712 passing, 1 skipped)
│   ├── core/               # Core model and analysis tests
│   │   ├── test_node.cpp
│   │   ├── test_member.cpp
│   │   ├── test_truss.cpp
│   │   └── test_analysis_engine.cpp
│   ├── cli/                # CLI layer tests (29 tests)
│   │   ├── test_analyze_command.cpp      # AnalyzeCommand tests (11)
│   │   ├── test_validate_command.cpp     # ValidateCommand tests (8)
│   │   ├── test_export_command.cpp       # ExportCommand tests (11)
│   │   ├── test_help_command.cpp         # HelpCommand tests
│   │   ├── test_console_presenter.cpp    # ConsolePresenter tests
│   │   └── test_argument_parser.cpp      # ArgumentParser tests (28)
│   ├── gui/                # GUI layer tests
│   │   └── test_TrussEditController_Advanced.cpp  # Advanced GUI tests (8)
│   ├── infrastructure/     # Infrastructure layer tests
│   │   ├── export/
│   │   │   └── test_export_advanced.cpp           # Export edge cases (8)
│   │   ├── io/
│   │   │   └── test_fileio_advanced.cpp           # FileIO edge cases (7)
│   │   ├── logging/
│   │   │   └── test_logger_advanced.cpp           # Logger tests (22)
│   │   └── validation/
│   │       └── test_validation_advanced.cpp       # Validation tests (19)
│   └── application/        # Application layer facade tests
│       ├── test_truss_application_service.cpp
│       └── test_analysis_application_service.cpp
│
├── integration/             # Integration tests (53 passing)
│   ├── test_e2e_workflows.cpp           # E2E workflow scenarios (7)
│   ├── test_cli_integration.cpp         # CLI workflows
│   ├── test_simple_truss_analysis.cpp   # Analysis workflows
│   └── test_gui_integration.cpp         # GUI integration
│
├── fixtures/                # Shared test data and helpers
│   ├── export_golden/      # Golden master files for export validation
│   └── test_data/          # JSON/XML test truss structures
│
└── test_gtest_integration.cpp  # GTest framework validation
```

## Test Statistics

### Overall Metrics

- **Total Tests:** 766 (713 unit + 53 integration)
- **Passing:** 765 (99.87%)
- **Skipped:** 1 (golden master validation)
- **Failed:** 0
- **Execution Time:** 148ms total
  - Unit tests: 143ms (0.20ms avg)
  - Integration tests: 5ms (0.09ms avg)

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
  - 458/459 tests passing (99.8% pass rate)
- **Target:** `unit_tests` (aggregate executable)

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

- `test_gtest_integration` - GTest framework validation
- `unit_tests` - All unit tests (single executable)
- `integration_tests` - All integration tests (single executable)
- `all_tests` - Convenience target to build all tests

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
- [Testing Strategy](../docs/refactoring/05-TESTING-STRATEGY.md)
