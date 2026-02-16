# Test Directory Structure

This directory contains all tests for the 2D Truss Analysis project, organized by layer and component using Google Test framework.

## Directory Structure

```
tests/
├── unit/                    # Unit tests (458 passing, 1 skipped)
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
│   └── application/        # Application layer facade tests
│       ├── test_truss_application_service.cpp
│       └── test_analysis_application_service.cpp
│
├── integration/             # Integration tests (multi-component workflows)
│   ├── test_cli_integration.cpp         # End-to-end CLI workflows
│   ├── test_simple_truss_analysis.cpp
│   ├── test_simple_integration.cpp
│   ├── test_working_integration.cpp
│   └── test_minimal_analysis.cpp
│
├── fixtures/                # Shared test data and helpers
│   ├── export_golden/      # Golden master files for export validation
│   └── test_data/          # JSON/XML test truss structures
│
└── test_gtest_integration.cpp  # GTest framework validation
```

## Test Organization Principles

### Unit Tests (`tests/unit/`)

- **Framework:** Google Test (GTest) with GMock for mocking
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
