# Testing Strategy: 2D Truss Analysis (Refactored)

**Date:** February 4, 2026  
**Framework:** Google Test (gtest/gmock)  
**Target Coverage:** 80%+

---

## 1. Testing Philosophy

**Priorities:**

1. **Computational Correctness** - Numerical accuracy is critical
2. **Test Pyramid** - Many unit tests, fewer integration tests, minimal E2E
3. **Fast Feedback** - Tests run in < 30 seconds
4. **Isolated Testing** - Each component tested independently
5. **Continuous Validation** - Tests run on every commit

---

## 2. Test Framework Migration

### 2.1 From Custom to Google Test

**Current:** Custom `TestFramework.hpp` (379 lines)  
**Target:** Google Test + Google Mock

**Migration Benefits:**

- Industry-standard framework
- IDE integration (VS Code, CLion)
- Advanced features (fixtures, parametrized tests, mocks)
- Better error messages
- JUnit XML output for CI/CD

### 2.2 Assertion Mapping

| Custom Framework         | Google Test                          |
| ------------------------ | ------------------------------------ |
| `ASSERT_TRUE(x)`         | `EXPECT_TRUE(x)` or `ASSERT_TRUE(x)` |
| `ASSERT_EQUAL(a, b)`     | `EXPECT_EQ(a, b)`                    |
| `ASSERT_NEAR(a, b, eps)` | `EXPECT_NEAR(a, b, eps)`             |
| `ASSERT_GT(a, b)`        | `EXPECT_GT(a, b)`                    |
| `ASSERT_THROWS(expr)`    | `EXPECT_THROW(expr, Exception)`      |

**Guidelines:**

- Use `EXPECT_*` for most assertions (test continues on failure)
- Use `ASSERT_*` when further testing makes no sense (test stops)

---

## 3. Test Organization

### 3.1 Directory Structure

```
tests/
├── unit/                  # Unit tests (isolated components)
│   ├── core/
│   │   ├── model/
│   │   │   ├── test_node.cpp
│   │   │   ├── test_member.cpp
│   │   │   ├── test_truss.cpp
│   │   │   └── test_value_objects.cpp
│   │   ├── analysis/
│   │   │   ├── test_stiffness_assembler.cpp
│   │   │   ├── test_boundary_condition_handler.cpp
│   │   │   └── test_direct_solver.cpp
│   │   └── validation/
│   │       └── test_truss_validator.cpp
│   ├── infrastructure/
│   │   ├── io/
│   │   ├── export/
│   │   └── logging/
│   └── utilities/
│
├── integration/           # Multi-component tests
│   ├── test_simple_truss_analysis.cpp
│   ├── test_bridge_analysis.cpp
│   └── test_export_workflow.cpp
│
├── system/                # End-to-end tests
│   ├── test_cli_workflow.cpp
│   └── test_complete_pipeline.cpp
│
├── performance/           # Benchmarks
│   ├── benchmark_small_truss.cpp
│   ├── benchmark_large_truss.cpp
│   └── benchmark_export.cpp
│
├── fixtures/              # Shared test data
│   ├── test_fixture.hpp
│   └── sample_data/
│
└── mocks/                 # Mock objects
    ├── mock_linear_solver.hpp
    └── mock_file_reader.hpp
```

### 3.2 Test Levels

**Unit Tests (70% of tests):**

- Test single class or function
- No dependencies on other components
- Fast execution (< 1ms per test)
- Use mocks for external dependencies

**Integration Tests (25% of tests):**

- Test multiple components working together
- Real implementations (minimal mocking)
- Moderate execution time (< 100ms per test)
- Validate interfaces between modules

**System Tests (5% of tests):**

- End-to-end workflows
- Complete application behavior
- Slower execution (< 1s per test)
- Validate user-facing features

---

## 4. Test Fixtures

### 4.1 Base Fixture

```cpp
// tests/fixtures/test_fixture.hpp
class TrussTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup
    }

    void TearDown() override {
        // Cleanup
    }

    // Helper methods
    Truss createSimpleTriangularTruss() {
        Truss truss("Triangle");
        auto n1 = truss.addNode(0, 0, SupportType::Pinned);
        auto n2 = truss.addNode(4, 0, SupportType::RollerY);
        auto n3 = truss.addNode(2, 3, SupportType::Free);
        truss.addMember(n1, n2, getSteelMaterial(), getStandardSection());
        truss.addMember(n1, n3, getSteelMaterial(), getStandardSection());
        truss.addMember(n2, n3, getSteelMaterial(), getStandardSection());
        truss.applyForce(n3->getId(), Force(0, -10000));
        return truss;
    }

    Truss createBridgeTruss() {
        // More complex truss for testing
    }

    MaterialProperties getSteelMaterial() {
        return MaterialProperties{
            .youngModulus = 200e9,
            .yieldStrength = 250e6,
            .density = 7850,
            .name = "Steel"
        };
    }

    SectionProperties getStandardSection() {
        return SectionProperties{
            .area = 0.01,
            .momentOfInertia = 1e-6,
            .type = "Square"
        };
    }

    // Numerical comparison helpers
    void expectNear(double a, double b, double tolerance = 1e-9) {
        EXPECT_NEAR(a, b, tolerance);
    }

    void expectVectorNear(const Eigen::VectorXd& a, const Eigen::VectorXd& b,
                         double tolerance = 1e-9) {
        ASSERT_EQ(a.size(), b.size());
        for (int i = 0; i < a.size(); ++i) {
            EXPECT_NEAR(a(i), b(i), tolerance) << "at index " << i;
        }
    }
};
```

### 4.2 Specialized Fixtures

```cpp
// For testing analysis components
class AnalysisTestFixture : public TrussTestFixture {
protected:
    std::unique_ptr<StiffnessAssembler> assembler;

    void SetUp() override {
        TrussTestFixture::SetUp();
        assembler = std::make_unique<StiffnessAssembler>();
    }
};

// For testing file I/O
class FileIOTestFixture : public ::testing::Test {
protected:
    std::filesystem::path tempDir;

    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "truss_test";
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }
};
```

---

## 5. Test Patterns and Examples

### 5.1 Unit Test Example

```cpp
// tests/unit/core/model/test_node.cpp
#include <gtest/gtest.h>
#include "core/model/node.hpp"

class NodeTest : public ::testing::Test {};

TEST_F(NodeTest, DefaultConstructorCreatesValidNode) {
    Node node(Point2D{0, 0}, SupportType::Free);

    EXPECT_TRUE(node.isValid());
    EXPECT_EQ(node.getSupportType(), SupportType::Free);
    EXPECT_EQ(node.getDegreesOfFreedom(), 2);
}

TEST_F(NodeTest, PinnedSupportHasZeroDOF) {
    Node node(Point2D{0, 0}, SupportType::Pinned);

    EXPECT_EQ(node.getDegreesOfFreedom(), 0);
    EXPECT_TRUE(node.isConstrained());
}

TEST_F(NodeTest, ForceApplicationWorks) {
    Node node(Point2D{0, 0}, SupportType::Free);
    Force force(100, -200);

    node.applyForce(force);

    EXPECT_EQ(node.getAppliedForces().size(), 1);
    EXPECT_DOUBLE_EQ(node.getAppliedForces()[0].magnitude(),
                     std::sqrt(100*100 + 200*200));
}
```

### 5.2 Integration Test Example

```cpp
// tests/integration/test_simple_truss_analysis.cpp
#include <gtest/gtest.h>
#include "fixtures/test_fixture.hpp"
#include "interface/truss_analysis_facade.hpp"

class SimpleAnalysisTest : public TrussTestFixture {};

TEST_F(SimpleAnalysisTest, TriangularTrussAnalysisProducesReasonableResults) {
    // Arrange
    Truss truss = createSimpleTriangularTruss();
    TrussAnalysisFacade facade;

    // Act
    AnalysisResults results = facade.analyzeInteractive(truss);

    // Assert
    EXPECT_TRUE(results.converged);
    EXPECT_GT(results.maxDisplacement, 0.0);
    EXPECT_LT(results.maxDisplacement, 0.1);  // Reasonable bounds
    EXPECT_EQ(results.memberForces.size(), 3);

    // Verify equilibrium (sum of reactions = applied load)
    double totalReactionY = 0.0;
    for (auto reaction : results.reactions) {
        if (/* Y-direction */) totalReactionY += reaction;
    }
    EXPECT_NEAR(totalReactionY, 10000, 1.0);  // Applied load = 10000 N
}
```

### 5.3 Parametrized Test Example

```cpp
// tests/unit/core/analysis/test_linear_solver.cpp
#include <gtest/gtest.h>

class LinearSolverTest : public ::testing::TestWithParam<SolverType> {};

TEST_P(LinearSolverTest, SolvesSimpleSystem) {
    SolverType type = GetParam();
    auto solver = SolverFactory::create(type);

    // Create simple 2x2 system: [2 1; 1 3]x = [1; 2]
    Eigen::SparseMatrix<double> A(2, 2);
    A.insert(0, 0) = 2; A.insert(0, 1) = 1;
    A.insert(1, 0) = 1; A.insert(1, 1) = 3;
    Eigen::VectorXd b(2); b << 1, 2;

    Eigen::VectorXd x = solver->solve(A, b);

    // Expected solution: x = [0.2, 0.6]
    EXPECT_NEAR(x(0), 0.2, 1e-9);
    EXPECT_NEAR(x(1), 0.6, 1e-9);
}

INSTANTIATE_TEST_SUITE_P(AllSolvers, LinearSolverTest,
    ::testing::Values(SolverType::Direct, SolverType::Iterative));
```

### 5.4 Mock Example

```cpp
// tests/mocks/mock_linear_solver.hpp
#include <gmock/gmock.h>
#include "core/analysis/linear_solver.hpp"

class MockLinearSolver : public ILinearSolver {
public:
    MOCK_METHOD(Eigen::VectorXd, solve,
                (const Eigen::SparseMatrix<double>& A,
                 const Eigen::VectorXd& b),
                (override));
};

// Usage in tests:
TEST_F(AnalysisOrchestratorTest, UsesMockSolver) {
    auto mockSolver = std::make_unique<MockLinearSolver>();

    Eigen::VectorXd expectedSolution(4);
    expectedSolution << 0.1, 0.2, 0.3, 0.4;

    EXPECT_CALL(*mockSolver, solve(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(expectedSolution));

    AnalysisOrchestrator orchestrator(/* inject mockSolver */);
    // Test orchestrator logic without actual solver computation
}
```

---

## 6. Coverage Goals

### 6.1 Target Coverage by Layer

| Layer             | Target Coverage | Priority |
| ----------------- | --------------- | -------- |
| Core (model)      | 95%             | Critical |
| Core (analysis)   | 90%             | Critical |
| Core (validation) | 85%             | High     |
| Infrastructure    | 75%             | Medium   |
| Interface         | 70%             | Medium   |
| CLI               | 60%             | Low      |
| GUI               | 50%             | Low      |
| **Overall**       | **80%**         | -        |

### 6.2 Coverage Measurement

```cmake
# CMakeLists.txt
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()
```

```bash
# Generate coverage report
cmake -B build -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

---

## 7. Performance Testing

### 7.1 Benchmark Framework

```cpp
// tests/performance/benchmark_small_truss.cpp
#include <gtest/gtest.h>
#include <chrono>

class TrussBenchmark : public ::testing::Test {
protected:
    template<typename Func>
    double measureTime(Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

TEST_F(TrussBenchmark, SmallTrussAnalysisUnder10ms) {
    Truss truss = createSmallTruss(10);  // 10 nodes
    AnalysisOrchestrator orchestrator;

    double time = measureTime([&]() {
        orchestrator.analyze(truss, AnalysisOptions{});
    });

    EXPECT_LT(time, 10.0) << "Analysis took " << time << " ms";
}

TEST_F(TrussBenchmark, MediumTrussAnalysisUnder100ms) {
    Truss truss = createMediumTruss(100);  // 100 nodes
    // ...
}
```

### 7.2 Performance Regression Testing

Track performance over time:

```bash
# Run benchmarks and save results
./build/performance_tests --gtest_output=json:bench_results.json

# Compare with baseline
python scripts/compare_benchmarks.py baseline.json bench_results.json
```

---

## 8. Test Execution Strategy

### 8.1 Local Development

```bash
# Quick test (unit tests only)
make test-unit              # ~5 seconds

# Full test suite
make test                   # ~30 seconds

# Specific test
./build/unit_tests --gtest_filter=NodeTest.*

# With coverage
make test-coverage
```

### 8.2 CI/CD Pipeline

```yaml
# .github/workflows/build-and-test.yml
name: Build and Test

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v3

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake libeigen3-dev libgtest-dev

      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
          cmake --build build -j$(nproc)

      - name: Run tests
        run: |
          cd build
          ctest --output-on-failure

      - name: Coverage
        if: matrix.build_type == 'Debug'
        run: |
          # Generate coverage report
          # Upload to Codecov
```

---

## 9. Test Data Management

### 9.1 Sample Data Files

```
tests/fixtures/sample_data/
├── simple_truss.json          # Valid simple truss
├── bridge_truss.json          # More complex structure
├── invalid_truss_unstable.json
├── invalid_truss_indeterminate.json
└── malformed.json             # Parse error test
```

### 9.2 Test Data Generation

```cpp
// tests/fixtures/test_data_generator.hpp
class TestDataGenerator {
public:
    static Truss generateRandomTruss(int numNodes, int seed = 42);
    static Truss generateGridTruss(int rows, int cols);
    static Truss generateBridgeTruss(int spans);
};
```

---

## 10. Validation Tests (Critical)

### 10.1 Numerical Correctness Tests

```cpp
// Validate against known analytical solutions
TEST_F(ValidationTest, SimpleBeamDeflection) {
    // Simple beam with point load at center
    // Analytical solution: δ = FL³/(48EI)
    Truss beam = createSimpleBeam(4.0);  // 4m span
    double F = 1000;  // 1000 N load

    AnalysisResults results = analyze(beam);

    double E = 200e9;
    double I = 1e-6;
    double L = 4.0;
    double analytical = (F * L*L*L) / (48 * E * I);

    EXPECT_NEAR(results.maxDisplacement, analytical, analytical * 0.01);  // 1% tolerance
}
```

### 10.2 Regression Tests

```cpp
// Ensure refactored code produces same results as original
TEST_F(RegressionTest, MatchesReferenceImplementation) {
    // Load reference results from v2.2.0
    ReferenceResults ref = loadReferenceResults("reference_v2.2.0.json");

    Truss truss = ref.truss;
    AnalysisResults results = facade.analyzeInteractive(truss);

    expectVectorNear(results.displacements, ref.displacements);
    expectVectorNear(results.reactions, ref.reactions);
    expectVectorNear(results.memberForces, ref.memberForces);
}
```

---

## 11. Testing Best Practices

### 11.1 Test Naming

```cpp
// Pattern: MethodName_StateUnderTest_ExpectedBehavior
TEST_F(NodeTest, AddForce_WhenNodeIsFree_IncreasesForceCount)
TEST_F(TrussTest, AddMember_WhenNodesAreValid_ReturnsMemberId)
TEST_F(SolverTest, Solve_WhenMatrixIsSingular_ThrowsException)
```

### 11.2 Test Independence

- Each test must be independent
- No shared state between tests
- Use fixtures for setup/teardown
- Tests can run in any order

### 11.3 Test Readability

```cpp
// Arrange-Act-Assert pattern
TEST_F(AnalysisTest, BasicWorkflow) {
    // Arrange
    Truss truss = createSimpleTriangularTruss();
    AnalysisOptions options{.useDirectSolver = true};

    // Act
    AnalysisResults results = orchestrator.analyze(truss, options);

    // Assert
    EXPECT_TRUE(results.converged);
    EXPECT_GT(results.maxDisplacement, 0.0);
}
```

---

## 12. Migration Checklist

- [ ] Google Test integrated in CMake
- [ ] All unit tests migrated
- [ ] All integration tests migrated
- [ ] Debug tests deleted
- [ ] Test fixtures created
- [ ] Mock objects implemented
- [ ] Custom TestFramework.hpp deleted
- [ ] Test organization complete (unit/integration/system)
- [ ] Coverage measurement configured
- [ ] Performance benchmarks created
- [ ] CI/CD pipeline updated
- [ ] Test documentation written

---

## 13. Success Criteria

✅ **All tests pass** with Google Test  
✅ **Coverage ≥ 80%** overall  
✅ **Critical paths ≥ 90%** coverage  
✅ **Tests run in < 30s** locally  
✅ **No test interdependencies**  
✅ **Numerical correctness validated**  
✅ **CI/CD integration complete**

---

**Document Status:** Ready for Implementation  
**Next:** Begin Phase 1 - Test Infrastructure Migration
