# Development Guide

## Overview

This document provides comprehensive information for developers contributing to the 2D Truss Analysis project. It covers testing frameworks, build systems, CI/CD pipelines, and development workflows.

## Testing Framework

### Unit Testing Architecture

Our project uses Google Test (GTest) framework with comprehensive mock-based testing for all layers:

- **Framework**: Google Test with GMock for mocking
- **Coverage**: 1603/1603 tests passing (100% pass rate)
- **Architecture**: Layer-specific test suites with focused mock-based testing
- **CLI Testing**: Command Pattern tests with simplified output mocking
- **Integration**: CMake CTest integration with parallel execution

### Test Categories

- **Unit Tests**: Layer-isolated tests with comprehensive mocking
- **CLI Tests**: Command execution, argument parsing, output formatting
- **Integration Tests**: End-to-end workflow validation
- **Application Tests**: Service layer and facade testing
- **Core Tests**: Domain logic and computational engine validation

### Test Structure

```
tests/
├── unit/
│   ├── application/        # TrussApplicationService, MaterialLibraryService, etc.
│   ├── cli/                # CLI command and argument parser tests
│   ├── core/
│   │   ├── analysis/       # Orchestrator, solver, stiffness assembler
│   │   ├── assembly/       # TrussAssembler tests
│   │   ├── model/          # Node, Member, Truss, Load tests
│   │   └── validation/     # TrussValidator tests
│   ├── gui/                # GUI layer tests (Qt: controllers, models, panels, widgets)
│   ├── infrastructure/     # Export, FileIO, logging tests
│   ├── interface/          # Facade adapter tests
│   └── utilities/          # Math and string utility tests
├── integration/             # End-to-end workflow tests
├── fixtures/                # Shared test data (golden masters, JSON/XML structures)
└── test_gtest_integration.cpp  # GTest framework validation
```

### Running Tests

#### Using CMake/CTest (Recommended)

```bash
# Configure with testing enabled
cmake -B build -DBUILD_TESTING=ON

# Build all tests
cmake --build build

# Run all tests with CTest
cd build && ctest --output-on-failure

# Run specific test
cd build && ctest -R test_Integration --verbose
```

#### Using CTest Directly

```bash
# Run all 5 CTest targets
cd build && ctest --output-on-failure

# Filter by label
ctest -L unit
ctest -L integration

# Run a specific executable directly
./build/tests/unit/unit_tests --gtest_filter="TrussTest.*"
./build/tests/unit/unit_tests_gui_widgets --gtest_filter="CanvasControllerTest.*"
```

### Test Framework Features

#### GTest Assertion Macros

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Basic assertions
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
EXPECT_EQ(expected, actual);
EXPECT_NE(value1, value2);

// Numerical comparisons
EXPECT_NEAR(expected, actual, tolerance);
EXPECT_GT(value1, value2);
EXPECT_LT(value1, value2);

// Exception handling
EXPECT_THROW(expression, ExceptionType);
EXPECT_NO_THROW(expression);
```

#### Test Fixture Pattern

````cpp
class TrussTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Build a minimal valid truss for each test
    }
    Truss truss;
};

TEST_F(TrussTest, AddNodeIncreasesCount) {
    truss.addNode(Node(1, 0.0, 0.0));
    EXPECT_EQ(truss.getNodeCount(), 1);
}

## Build System

### Production Makefile Wrapper (Recommended)

The project provides a production-grade Makefile that wraps CMake and provides a streamlined developer experience:

```bash
# Show all available targets
make help

# Build targets
make build              # Build release version (optimized)
make debug              # Build debug version (with symbols)
make rebuild            # Clean and rebuild release

# Test targets
make test               # Run all tests (release build)
make test-debug         # Run tests in debug mode
make coverage           # Generate coverage report with lcov
make coverage-open      # Generate and open coverage in browser

# Code quality targets
make format             # Format C++ code with clang-format
make format-docs        # Format Markdown docs with Prettier
make format-yaml        # Format YAML files with Prettier
make format-all         # Format everything (C++, docs, YAML)
make format-check-all   # Check formatting (CI-friendly)
make lint               # Run clang-tidy static analysis
make static-analysis    # Run cppcheck analysis

# Development targets
make info               # Show build system configuration
make clean              # Clean release build
make clean-all          # Clean all build artifacts

# CI/CD targets
make ci                 # CI pipeline (build + test + format check)
make ci-full            # Full CI (build + test + coverage + all checks)
````

**Key Features:**

- **Auto-detection**: Prefers Ninja, falls back to Make
- **Parallel builds**: Automatically detects CPU cores
- **Isolated directories**: `build/`, `build_debug/`, `build_coverage/`
- **Generator-agnostic**: Works with both Ninja and Make backends
- **Self-documenting**: Comprehensive help with categorized targets

### Direct CMake Usage (Alternative)

```bash
# Standard build types
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Testing configuration
cmake -B build -DBUILD_TESTING=ON

# Compiler selection
cmake -B build -DCMAKE_CXX_COMPILER=g++
cmake -B build -DCMAKE_CXX_COMPILER=clang++

# Coverage build
cmake -B build_coverage -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build_coverage --target coverage
```

### CMake Test Integration

The build system automatically discovers and configures unit tests:

- Each `.cpp` file in `tests/unit/` becomes a CTest
- Tests are compiled with `DEBUG_TESTING` flag for debug interface access
- Automatic dependency linking with `TrussCore` library
- Timeout and working directory configuration

## CI/CD Pipeline

### GitHub Actions Workflow

Our comprehensive CI/CD pipeline (`/.github/workflows/ci.yml`) includes:

#### Test Matrix

- **Compilers**: GCC-11, Clang-13
- **Build Types**: Debug, Release
- **Platforms**: Ubuntu Latest (Linux-only)

#### Pipeline Stages

1. **Build and Test**
   - Dependency installation (Eigen3, CMake)
   - Multi-compiler build verification
   - Unit test execution
   - Integration test validation

2. **Static Analysis**
   - **cppcheck**: Comprehensive code analysis
   - **clang-tidy**: Modern C++ best practices
   - Configurable rule sets for different analysis types

3. **Performance Testing**
   - **Valgrind**: Memory leak detection
   - Performance benchmarking
   - Execution time measurement

4. **Documentation Generation**
   - **Doxygen**: API documentation generation
   - Automatic deployment to GitHub Pages
   - Cross-referenced source code documentation

### Artifact Collection

The CI system automatically collects:

- Test executables and results
- Static analysis reports (XML/TXT formats)
- Performance benchmark data
- Memory analysis reports
- Generated documentation

### Running CI Locally

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install cmake build-essential libeigen3-dev \
                     cppcheck clang-tidy valgrind doxygen

# Replicate CI build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

## Debug Interface

### Analysis Engine Debug Methods

For advanced testing, the `AnalysisEngine` provides debug access:

```cpp
#ifdef DEBUG_TESTING
// DOF analysis
auto freeDofs = engine.getFreeDofIndicesPublic(truss);
auto constrainedDofs = engine.getConstrainedDofIndicesPublic(truss);

// Matrix operations
MatrixXd Kff = engine.extractFreeStiffnessMatrixPublic(K, freeDofs);
VectorXd Ff = engine.extractFreeLoadVectorPublic(F, freeDofs);

// Solution methods
VectorXd solution = engine.solveWithLDLTPublic(Kff, Ff);
VectorXd fullSolution = engine.expandSolutionVectorPublic(
    solution, freeDofs, totalDofs);
#endif
```

### Debug Compilation

Debug interface is enabled automatically for test builds:

```bash
# CMake automatically adds -DDEBUG_TESTING for unit tests
cmake -B build -DBUILD_TESTING=ON
```

## Development Workflow

### Contributing Guidelines

1. **Feature Development**

   ```bash
   git checkout -b feature/your-feature-name
   # Implement feature
   # Add comprehensive tests
   git commit -m "feat: descriptive commit message"
   ```

2. **Testing Requirements**
   - All new functionality must include unit tests
   - Integration tests for user-facing features
   - Performance tests for optimization-critical code
   - Memory leak verification for resource management

3. **Code Quality Standards**
   - Follow existing code style and patterns
   - Pass all static analysis checks
   - Maintain or improve test coverage
   - Document public APIs with Doxygen comments

### Pre-commit Checklist

```bash
# 1. Format all code
make format-all

# 2. Build successfully
make build

# 3. Pass all tests
make test

# 4. Verify formatting (CI check)
make format-check-all

# 5. Run static analysis
make lint
make static-analysis

# 6. Generate coverage (optional)
make coverage

# Or run the full CI pipeline locally
make ci-full
```

**Automated Code Quality:**

- **clang-format**: C++ code formatting (`.clang-format` configuration)
- **Prettier**: Markdown and YAML formatting (`.prettierrc.yaml` configuration)
- **clang-tidy**: C++ static analysis (`.clang-tidy` configuration)
- **EditorConfig**: Baseline formatting for all file types (`.editorconfig`)
- **cppcheck**: Additional static analysis

**cppcheck Notes:**

- The Makefile `static-analysis` target defines Qt macros (`slots`, `signals`, `Q_OBJECT`, `emit`) to avoid false positives.
- Output is written to `cppcheck-report.txt` at the repository root.

**Configuration Files:**

- `.clang-format` - C++ code style (LLVM-based with 100-char lines)
- `.clang-tidy` - C++ static analysis rules (400+ lines, 8 check categories)
- `.prettierrc.yaml` - Markdown/YAML formatting (100-char prose, 2-space indent)
- `.prettierignore` - Files excluded from Prettier
- `.editorconfig` - Universal baseline (200+ lines, 25+ file types)
- `package.json` - NPM scripts for Prettier

## Performance Considerations

### Optimization Flags

```bash
# Release builds use aggressive optimization
cmake -B build -DCMAKE_BUILD_TYPE=Release
# Adds: -O3 -march=native -DNDEBUG
```

### Profiling and Benchmarking

```bash
# Build with profiling
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Profile execution
perf record ./build/TrussAnalysisCLI analyze -f example.json -v
perf report

# Memory profiling
valgrind --tool=massif ./build/test_integration
ms_print massif.out.* | head -30
```

## Troubleshooting

### Common Build Issues

1. **Missing Eigen3**

   ```bash
   # Ubuntu/Debian
   sudo apt-get install libeigen3-dev

   # Fedora
   sudo dnf install eigen3-devel
   ```

2. **Qt6 Issues**

   ```bash
   # Ensure Qt6 is properly installed
   sudo apt-get install qt6-base-dev qt6-tools-dev
   ```

3. **Test Compilation Failures**
   - Ensure `DEBUG_TESTING` flag is set for test builds
   - Verify `TrussCore` library is built successfully
   - Check include paths in CMake configuration

### Test Failures

1. **Numerical Precision Issues**
   - Adjust tolerance in `ASSERT_NEAR` calls
   - Consider platform-specific floating-point differences
   - Use relative rather than absolute tolerances for large values

2. **Memory-Related Failures**
   - Run with Valgrind to detect leaks
   - Check for uninitialized variables
   - Verify proper RAII usage

## Future Enhancements

### Planned Testing Improvements

1. **Benchmark Testing Framework**
   - Automated performance regression detection
   - Historical performance tracking
   - Comparative analysis across platforms

2. **Fuzz Testing Integration**
   - Automated input generation for robustness testing
   - Edge case discovery for numerical algorithms
   - Integration with CI pipeline

3. **Property-Based Testing**
   - Mathematical property verification
   - Structural analysis invariant checking
   - Automated test case generation

### CI/CD Enhancements

1. **Extended Linux Support**
   - Additional Linux distributions (Fedora, Arch, Debian)
   - ARM64 architecture support
   - Container-based builds

2. **Advanced Analysis**
   - Code coverage reporting
   - Security vulnerability scanning
   - Dependency vulnerability tracking

3. **Deployment Automation**
   - Release artifact generation
   - Package manager integration
   - Documentation site updates
