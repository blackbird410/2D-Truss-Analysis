# Refactoring Master Plan: 2D Truss Analysis C++ Project

**Date:** February 4, 2026 (Created) | Updated: February 14, 2026  
**Project:** 2D Truss Analysis (Linux-Only Refactor)  
**Current Version:** 2.2.0 → 3.0.0-dev  
**Progress:** Phases 0-4 Complete ✅ | Phases 5-6 Remaining  
**Estimated Duration:** 10-12 weeks (part-time) or 3-4 weeks (full-time)

---

## Executive Summary

This master plan outlines a phased, risk-managed approach to refactoring the 2D Truss Analysis C++ project from a practice-grade codebase into a production-quality, portfolio-ready application. The refactoring targets Linux-only deployment, eliminates platform-specific bloat, implements professional software engineering practices, and establishes comprehensive testing infrastructure.

**Key Objectives:**

1. Remove all macOS-specific code and artifacts
2. Restructure codebase following professional conventions
3. Decompose monolithic classes into focused, testable modules
4. Migrate from custom test framework to Google Test
5. Implement professional build system and containerization
6. Establish comprehensive documentation

**Approach:** Incremental, test-driven refactoring with continuous validation

---

## 1. Refactoring Phases Overview

The refactoring is divided into 7 phases, each with specific goals, deliverables, and validation criteria:

| Phase | Name                        | Duration  | Risk   | Status          | Completion Date |
| ----- | --------------------------- | --------- | ------ | --------------- | --------------- |
| **0** | **Foundation & Cleanup**    | 1 week    | Low    | ✅ **COMPLETE** | Feb 4, 2026     |
| **1** | **Test Infrastructure**     | 1-2 weeks | Medium | ✅ **COMPLETE** | Feb 6, 2026     |
| **2** | **Core Refactoring**        | 2-3 weeks | High   | ✅ **COMPLETE** | Feb 9, 2026     |
| **3** | **Infrastructure Layer**    | 1-2 weeks | Medium | ✅ **COMPLETE** | Feb 13, 2026    |
| **4** | **Interface & Application** | 1-2 weeks | Low    | ✅ **COMPLETE** | Feb 14, 2026    |
| **5** | **Build & Deployment**      | 1 week    | Medium | ⏳ Pending      | -               |
| **6** | **Documentation & Polish**  | 1-2 weeks | Low    | ⏳ Pending      | -               |

**Progress:** 5/7 phases complete (71%) | **Elapsed Time:** 10 days | **Remaining:** ~2-3 weeks

---

## 2. Phase 0: Foundation & Cleanup

**Objective:** Establish clean foundation by removing bloat and organizing structure

**Duration:** 1 week (20-25 hours)

### 2.1 Tasks

#### Task 0.1: Create Documentation Structure

**Effort:** 2 hours  
**Risk:** Low

**Actions:**

1. Create `docs/` directory structure
2. Move existing documentation to appropriate locations
3. Create documentation index (docs/README.md)
4. Create `docs/refactoring/` for refactoring artifacts
5. Create `docs/work-logs/` for tracking changes

**Deliverables:**

- Complete `docs/` hierarchy
- Updated documentation index

**Validation:**

- [ ] All documentation in appropriate directories
- [ ] No documentation files in root (except README, LICENSE, CONTRIBUTING)

#### Task 0.2: Remove macOS-Specific Artifacts

**Effort:** 3 hours  
**Risk:** Low

**Actions:**

1. **Delete scripts (9 files):**
   - `install_macos.sh`
   - `build_and_package.sh`
   - `build_dmg.sh`
   - `fix_dmg_signing.sh`
   - `fix_dmg_signature.sh`
   - `sign_app.sh`
   - `sign_adhoc.sh`
   - `create_keychain_cert.sh`
   - `verify_signing.sh`

2. **Delete documentation (3+ files):**
   - `INSTALL_MACOS.md`
   - `CODE_SIGNING_SOLUTIONS.md`
   - Remove macOS sections from `QUICK_INSTALL.md`
   - Remove macOS sections from `README.md`

3. **Delete build artifacts:**
   - `2D_Truss_Analysis-*.dmg` files
   - `Info.plist.in`

4. **Clean CMakeLists.txt:**
   - Remove `if(APPLE)` blocks
   - Remove MACOSX_BUNDLE properties
   - Remove code signing logic

**Deliverables:**

- Zero macOS-specific files
- Clean CMakeLists.txt (Linux-only)

**Validation:**

- [ ] grep -r "macOS\|darwin\|APPLE" returns no results in code files
- [ ] Project builds on Linux
- [ ] No DMG-related files exist

#### Task 0.3: Update Git Ignore and Clean Repository

**Effort:** 2 hours  
**Risk:** Low

**Actions:**

1. Update `.gitignore` to exclude:
   - `build/` and `build_*/`
   - `install/`
   - Test binaries (`tests/unit/test_*` executables)
   - Generated test results (`test_results.*`)
   - IDE files (`.vscode/`, `.idea/`, `.DS_Store`)
   - Generated documentation (`docs/api/`)

2. Remove tracked build artifacts:

   ```bash
   git rm -r build/ build_test/
   git rm tests/unit/test_* (binaries only)
   git rm test_results.*
   ```

3. Create `.dockerignore`

**Deliverables:**

- Comprehensive `.gitignore`
- `.dockerignore`
- Clean Git repository

**Validation:**

- [ ] `git status` shows no build artifacts
- [ ] `build/` directory is ignored
- [ ] No test binaries tracked

#### Task 0.4: Archive Obsolete Documentation

**Effort:** 1 hour  
**Risk:** Low

**Actions:**

1. Create `docs/archive/` directory
2. Move to archive:
   - `FEATURE_ANNOUNCEMENT.md`
   - `PR_EMOJI_REMOVAL.md`
   - `UI_ACCESSIBILITY_FIXES_SUMMARY.md`
   - `LINUX_DISPLAY_FIXES_SUMMARY.md`
   - Old release notes (keep latest in root)

**Deliverables:**

- `docs/archive/` with historical documents
- Clean root directory

**Validation:**

- [ ] Root has <10 markdown files
- [ ] Historical documents accessible but not prominent

#### Task 0.5: Create Initial Directory Structure

**Effort:** 2 hours  
**Risk:** Low

**Actions:**

1. Create all directories from proposed structure:

   ```bash
   mkdir -p {docs,scripts,config,docker,examples,data}
   mkdir -p src/{cli,gui,interface,core,infrastructure,utilities}
   mkdir -p src/core/{model,analysis,validation}
   mkdir -p src/infrastructure/{io,export,logging,config}
   mkdir -p tests/{unit,integration,system,performance,fixtures,mocks}
   mkdir -p include/truss/{core,analysis,export}
   ```

2. Add `.gitkeep` files to empty directories

**Deliverables:**

- Complete directory structure (empty)

**Validation:**

- [ ] All directories exist
- [ ] Directory structure matches proposal

#### Task 0.6: Create Initial Work Log

**Effort:** 1 hour  
**Risk:** Low

**Actions:**

1. Create `docs/work-logs/2026-02-04-phase-0-foundation.md`
2. Document all changes made in Phase 0
3. Include rationale and decisions

**Deliverables:**

- First work log entry

### 2.2 Phase 0 Summary

**Total Effort:** 11 hours

**Deliverables:**

- ✅ Clean, organized documentation structure
- ✅ Zero macOS-specific code
- ✅ Clean Git repository
- ✅ Complete directory structure
- ✅ Updated .gitignore and .dockerignore
- ✅ First work log entry

**Validation Criteria:**

- [ ] Project builds successfully on Linux
- [ ] All tests pass (existing tests)
- [ ] No macOS references in codebase
- [ ] Git repository is clean
- [ ] Documentation is organized

**Risk Mitigation:**

- Low-risk phase, mostly deletions and moves
- No code changes yet
- Easy to revert if needed

---

## 3. Phase 1: Test Infrastructure Migration

**Objective:** Migrate from custom TestFramework.hpp to Google Test

**Duration:** 1-2 weeks (25-35 hours)

**Risk:** Medium (test framework changes can introduce bugs)

### 3.1 Tasks

#### Task 1.1: Set Up Google Test

**Effort:** 3 hours  
**Risk:** Low

**Actions:**

1. Add Google Test dependency to CMakeLists.txt:

   ```cmake
   find_package(GTest REQUIRED)
   include(GoogleTest)
   ```

2. Update vcpkg.json or install system-wide:

   ```json
   {
     "dependencies": ["gtest", "qt6", "eigen3"]
   }
   ```

3. Create `tests/CMakeLists.txt` for test configuration

4. Create `tests/test_main.cpp`:
   ```cpp
   #include <gtest/gtest.h>
   int main(int argc, char **argv) {
       ::testing::InitGoogleTest(&argc, argv);
       return RUN_ALL_TESTS();
   }
   ```

**Deliverables:**

- Google Test integrated in build system
- Test main file

**Validation:**

- [ ] Google Test found by CMake
- [ ] Test main compiles

#### Task 1.2: Create Test Fixtures

**Effort:** 4 hours  
**Risk:** Low

**Actions:**

1. Create `tests/fixtures/test_fixture.hpp`:

   ```cpp
   class TrussTestFixture : public ::testing::Test {
   protected:
       void SetUp() override;
       void TearDown() override;

       // Helper methods
       Truss createSimpleTriangularTruss();
       Truss createBridgeTruss();
       MaterialProperties getSteelProperties();
       SectionProperties getStandardSection();
   };
   ```

2. Implement common test helpers

**Deliverables:**

- Reusable test fixtures
- Common test data generators

#### Task 1.3: Migrate Unit Tests (Core Classes)

**Effort:** 12 hours  
**Risk:** Medium

**Priority Order:**

1. `test_Node.cpp` → `tests/unit/core/model/test_node.cpp`
2. `test_Member.cpp` → `tests/unit/core/model/test_member.cpp`
3. `test_Truss.cpp` → `tests/unit/core/model/test_truss.cpp`

**Migration Pattern:**

```cpp
// Old (TestFramework.hpp)
void test_node_creation() {
    Node node(0, 0, SupportType::Free);
    ASSERT_TRUE(node.isValid());
}
REGISTER_TEST(test_node_creation);

// New (Google Test)
TEST_F(TrussTestFixture, NodeCreation) {
    Node node(0, 0, SupportType::Free);
    EXPECT_TRUE(node.isValid());
}
```

**Conversion Table:**
| Old (TestFramework) | New (Google Test) |
|---------------------|-------------------|
| `ASSERT_TRUE(x)` | `EXPECT_TRUE(x)` or `ASSERT_TRUE(x)` |
| `ASSERT_FALSE(x)` | `EXPECT_FALSE(x)` or `ASSERT_FALSE(x)` |
| `ASSERT_EQUAL(a, b)` | `EXPECT_EQ(a, b)` or `ASSERT_EQ(a, b)` |
| `ASSERT_NEAR(a, b, eps)` | `EXPECT_NEAR(a, b, eps)` |
| `ASSERT_GT(a, b)` | `EXPECT_GT(a, b)` |
| `ASSERT_THROWS(x)` | `EXPECT_THROW(x, ExceptionType)` |

**Actions:**

1. Convert each test file one at a time
2. Run migrated tests immediately
3. Keep old tests until new tests pass
4. Delete old test files after successful migration

**Deliverables:**

- All core unit tests migrated to Google Test

**Validation:**

- [ ] All migrated tests pass
- [ ] Test coverage maintained or improved

#### Task 1.4: Migrate Integration Tests

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Identify true integration tests vs. debug tests
2. Migrate integration tests:
   - `test_Integration.cpp` → `tests/integration/test_simple_truss_analysis.cpp`
   - `test_SimpleIntegration.cpp` → `tests/integration/test_basic_workflow.cpp`
   - `test_WorkingIntegration.cpp` → `tests/integration/test_complete_analysis.cpp`

3. **Delete debug tests** (not production-ready):
   - `test_ConstraintDebug.cpp`
   - `test_DebugAnalysis.cpp`
   - `test_FreeSystemDebug.cpp`
   - `test_MemberDebug.cpp`
   - `test_MinimalAnalysis.cpp`
   - `test_PointerAnalysis.cpp`

**Deliverables:**

- Clean integration test suite
- Debug tests removed

**Validation:**

- [ ] Integration tests pass
- [ ] No debug tests remain

#### Task 1.5: Create Mock Objects

**Effort:** 4 hours  
**Risk:** Low

**Actions:**

1. Create `tests/mocks/mock_linear_solver.hpp`:

   ```cpp
   class MockLinearSolver : public ILinearSolver {
   public:
       MOCK_METHOD(Eigen::VectorXd, solve,
                   (const Eigen::SparseMatrix<double>&,
                    const Eigen::VectorXd&),
                   (override));
   };
   ```

2. Create other mocks as needed:
   - `mock_file_reader.hpp`
   - `mock_exporter.hpp`

**Deliverables:**

- Mock object library

**Validation:**

- [ ] Mocks compile
- [ ] Mocks used in tests

#### Task 1.6: Update CMake Test Configuration

**Effort:** 3 hours  
**Risk:** Low

**Actions:**

1. Create separate CMake targets for test levels:

   ```cmake
   # Unit tests
   add_executable(unit_tests
       tests/unit/core/model/test_node.cpp
       tests/unit/core/model/test_member.cpp
       ...
   )
   target_link_libraries(unit_tests PRIVATE
       TrussCore
       GTest::gtest
       GTest::gtest_main
   )
   gtest_discover_tests(unit_tests)

   # Integration tests
   add_executable(integration_tests ...)

   # System tests
   add_executable(system_tests ...)
   ```

2. Delete old test runner: `tests/run_all_tests.sh`

3. Create convenience test targets:
   ```cmake
   add_custom_target(test-unit
       COMMAND ${CMAKE_CTEST_COMMAND} -L unit
   )
   add_custom_target(test-integration
       COMMAND ${CMAKE_CTEST_COMMAND} -L integration
   )
   ```

**Deliverables:**

- Clean CMake test configuration
- CTest labels for test levels

**Validation:**

- [ ] `ctest -L unit` runs only unit tests
- [ ] `ctest -L integration` runs only integration tests
- [ ] All tests pass

#### Task 1.7: Delete Custom Test Framework

**Effort:** 1 hour  
**Risk:** Low

**Actions:**

1. Remove `tests/TestFramework.hpp` (379 lines)
2. Update any documentation referencing old framework

**Deliverables:**

- Custom test framework deleted

**Validation:**

- [ ] No references to TestFramework.hpp
- [ ] All tests use Google Test

### 3.2 Phase 1 Summary

**Total Effort:** 33 hours

**Deliverables:**

- ✅ Google Test framework integrated
- ✅ All tests migrated to Google Test
- ✅ Mock objects created
- ✅ Debug tests removed
- ✅ Clean test organization
- ✅ Custom TestFramework.hpp deleted

**Validation Criteria:**

- [ ] All tests pass with Google Test
- [ ] Test coverage ≥ previous coverage
- [ ] No custom test framework remains
- [ ] Tests organized by level (unit/integration/system)

**Risk Mitigation:**

- Migrate incrementally, test-by-test
- Keep old tests until new tests pass
- Run tests after each migration step
- Document any test behavior changes

---

## 4. Phase 2: Core Domain Refactoring

**Objective:** Decompose monolithic classes and refactor core domain logic

**Duration:** 2-3 weeks (40-50 hours)

**Risk:** High (touches computational core)

### 4.1 Tasks

#### Task 2.1: Introduce Value Objects

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Create `src/core/model/value_objects.hpp`:

   ```cpp
   struct Point2D {
       double x, y;
       double distanceTo(const Point2D& other) const;
   };

   class Force {
       double fx_, fy_;
   public:
       Force(double fx, double fy);
       double magnitude() const;
       double direction() const;
       // Immutable
   };

   class Displacement {
       double ux_, uy_;
   public:
       Displacement(double ux, double uy);
       double magnitude() const;
       // Immutable
   };
   ```

2. Update Node and Member to use value objects

3. Write tests for value objects

**Deliverables:**

- Value object classes
- Tests for value objects

**Validation:**

- [ ] Value object tests pass
- [ ] Node/Member updated to use value objects
- [ ] Existing tests still pass

#### Task 2.2: Refactor Node Class

**Effort:** 4 hours  
**Risk:** Medium

**Actions:**

1. Move `src/core/Node.*` → `src/core/model/node.*`
2. Update to use Point2D and Force value objects
3. Add behavior methods:

   ```cpp
   bool isConstrained() const;
   int getDegreesOfFreedom() const;
   std::vector<int> getGlobalDOFs() const;
   ```

4. Update tests

**Deliverables:**

- Refactored Node class
- Updated tests

**Validation:**

- [ ] Node tests pass
- [ ] No regressions in dependent tests

#### Task 2.3: Refactor Member Class

**Effort:** 4 hours  
**Risk:** Medium

**Actions:**

1. Move `src/core/Member.*` → `src/core/model/member.*`
2. Extract material and section properties:

   ```cpp
   // src/core/model/material.hpp
   struct MaterialProperties {
       double youngModulus;
       double yieldStrength;
       double density;
       std::string name;
   };

   // src/core/model/section.hpp
   struct SectionProperties {
       double area;
       double momentOfInertia;
       std::string type;
   };
   ```

3. Member stores references to material and section
4. Add computed properties:

   ```cpp
   double getLength() const;
   double getAxialStiffness() const;
   Eigen::Matrix4d getLocalStiffnessMatrix() const;
   ```

5. Update tests

**Deliverables:**

- Refactored Member class
- Material and Section types
- Updated tests

**Validation:**

- [ ] Member tests pass
- [ ] Material/section types work correctly

#### Task 2.4: Refactor Truss Class (Aggregate Root)

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Move `src/core/Truss.*` → `src/core/model/truss.*`
2. Enforce aggregate root pattern:
   - All modifications go through Truss
   - Nodes and Members not directly modifiable
3. Update storage to use smart pointers:

   ```cpp
   std::vector<std::unique_ptr<Node>> nodes_;
   std::vector<std::unique_ptr<Member>> members_;
   std::unordered_map<NodeId, Node*> nodeIndex_;
   ```

4. Add query methods:

   ```cpp
   const Node& getNode(NodeId id) const;
   std::vector<Member*> getMembersConnectedTo(NodeId node) const;
   ```

5. Update tests

**Deliverables:**

- Refactored Truss class
- Updated tests

**Validation:**

- [ ] Truss tests pass
- [ ] Aggregate root enforced

#### Task 2.5: Decompose AnalysisEngine (Part 1: Stiffness Assembly)

**Effort:** 8 hours  
**Risk:** High

**Actions:**

1. Create `src/core/analysis/stiffness_assembler.hpp`:

   ```cpp
   class StiffnessAssembler {
   public:
       Eigen::SparseMatrix<double> assembleGlobalStiffness(
           const Truss& truss
       );
   private:
       Eigen::Matrix4d computeLocalStiffness(const Member& member);
       Eigen::Matrix4d computeTransformationMatrix(const Member& member);
       void assembleMemberStiffness(
           const Member& member,
           Eigen::SparseMatrix<double>& K
       );
   };
   ```

2. Extract stiffness assembly logic from AnalysisEngine
3. Write comprehensive unit tests
4. Validate against original implementation

**Deliverables:**

- StiffnessAssembler class
- Unit tests

**Validation:**

- [ ] StiffnessAssembler tests pass
- [ ] Results match original implementation (numerical comparison)

#### Task 2.6: Decompose AnalysisEngine (Part 2: Boundary Conditions)

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Create `src/core/analysis/boundary_condition_handler.hpp`:

   ```cpp
   class BoundaryConditionHandler {
   public:
       struct ConstrainedSystem {
           Eigen::SparseMatrix<double> Kff;
           Eigen::VectorXd Ff;
           std::vector<int> constrainedDOFs;
       };

       ConstrainedSystem applyBoundaryConditions(
           const Eigen::SparseMatrix<double>& K,
           const Eigen::VectorXd& F,
           const Truss& truss
       );
   };
   ```

2. Extract boundary condition logic
3. Write unit tests

**Deliverables:**

- BoundaryConditionHandler class
- Unit tests

**Validation:**

- [ ] BoundaryConditionHandler tests pass
- [ ] Results match original implementation

#### Task 2.7: Decompose AnalysisEngine (Part 3: Linear Solver)

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Create solver interface:

   ```cpp
   // src/core/analysis/linear_solver.hpp
   class ILinearSolver {
   public:
       virtual ~ILinearSolver() = default;
       virtual Eigen::VectorXd solve(
           const Eigen::SparseMatrix<double>& A,
           const Eigen::VectorXd& b
       ) = 0;
   };
   ```

2. Implement concrete solvers:

   ```cpp
   // src/core/analysis/direct_solver.hpp
   class DirectSolver : public ILinearSolver {
       Eigen::SparseLU<Eigen::SparseMatrix<double>> solver_;
       // ...
   };

   // src/core/analysis/iterative_solver.hpp
   class IterativeSolver : public ILinearSolver {
       Eigen::ConjugateGradient<Eigen::SparseMatrix<double>> solver_;
       // ...
   };
   ```

3. Create solver factory:

   ```cpp
   class SolverFactory {
   public:
       static std::unique_ptr<ILinearSolver> create(SolverType type);
   };
   ```

4. Write tests for each solver

**Deliverables:**

- ILinearSolver interface
- DirectSolver and IterativeSolver implementations
- SolverFactory
- Unit tests

**Validation:**

- [ ] Both solvers produce correct results
- [ ] Factory creates correct solver types

#### Task 2.8: Create Analysis Orchestrator

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Create `src/core/analysis/analysis_orchestrator.hpp`:

   ```cpp
   class AnalysisOrchestrator {
       std::unique_ptr<StiffnessAssembler> stiffnessAssembler_;
       std::unique_ptr<BoundaryConditionHandler> bcHandler_;
       std::unique_ptr<ILinearSolver> solver_;
       std::unique_ptr<ResultsProcessor> resultsProcessor_;
   public:
       AnalysisResults analyze(
           const Truss& truss,
           const AnalysisOptions& options
       );
   };
   ```

2. Orchestrate workflow:
   - Assemble stiffness matrix
   - Apply boundary conditions
   - Solve linear system
   - Post-process results

3. Inject dependencies via constructor

4. Write integration tests

**Deliverables:**

- AnalysisOrchestrator class
- Integration tests

**Validation:**

- [ ] AnalysisOrchestrator produces same results as old AnalysisEngine
- [ ] All integration tests pass

#### Task 2.9: Delete Old AnalysisEngine

**Effort:** 2 hours  
**Risk:** Low

**Actions:**

1. Verify all functionality migrated
2. Delete `src/core/AnalysisEngine.*`
3. Update all references to use AnalysisOrchestrator

**Deliverables:**

- Old AnalysisEngine removed

**Validation:**

- [ ] No references to old AnalysisEngine
- [ ] All tests pass

### 4.2 Phase 2 Summary

**Total Effort:** 48 hours

**Deliverables:**

- ✅ Value objects (Force, Displacement, Point2D)
- ✅ Refactored Node, Member, Truss classes
- ✅ Decomposed analysis components (4 classes)
- ✅ Analysis orchestrator
- ✅ Old AnalysisEngine deleted

**Validation Criteria:**

- [ ] All unit tests pass
- [ ] All integration tests pass
- [ ] Numerical results match original implementation
- [ ] No regressions in functionality

**Risk Mitigation:**

- **Critical:** Validate numerical correctness at each step
- Keep old implementation until new one verified
- Use extensive numerical comparison tests
- Refactor incrementally, one class at a time

---

## 5. Phase 3: Infrastructure Layer Refactoring

**Objective:** Refactor I/O, export, logging, and configuration

**Duration:** 1-2 weeks (25-35 hours)

**Risk:** Medium

### 5.1 Tasks

#### Task 3.1: Refactor ResultsExporter (Strategy Pattern)

**Effort:** 10 hours  
**Risk:** Medium

**Actions:**

1. Create exporter interface:

   ```cpp
   // src/infrastructure/export/exporter.hpp
   class IResultsExporter {
   public:
       virtual ~IResultsExporter() = default;
       virtual bool exportResults(
           const AnalysisResults& results,
           const std::filesystem::path& path
       ) = 0;
   };
   ```

2. Implement concrete exporters (one per format):
   - `csv_exporter.hpp/.cpp`
   - `json_exporter.hpp/.cpp`
   - `html_exporter.hpp/.cpp`
   - `latex_exporter.hpp/.cpp`
   - `xml_exporter.hpp/.cpp`

3. Create factory:

   ```cpp
   class ExporterFactory {
   public:
       static std::unique_ptr<IResultsExporter> create(ExportFormat format);
   };
   ```

4. Write tests for each exporter

5. Delete old `src/core/ResultsExporter.*`

**Deliverables:**

- IResultsExporter interface
- 5 concrete exporter classes
- ExporterFactory
- Unit tests
- Old ResultsExporter deleted

**Validation:**

- [ ] Each exporter produces valid output
- [ ] Output format matches original implementation
- [ ] All exporter tests pass

#### Task 3.2: Implement File I/O Services

**Effort:** 8 hours  
**Risk:** Medium

**Actions:**

1. Create reader interface:

   ```cpp
   // src/infrastructure/io/file_reader.hpp
   class IFileReader {
   public:
       virtual ~IFileReader() = default;
       virtual Truss read(const std::filesystem::path& path) = 0;
   };
   ```

2. Implement concrete readers:
   - `json_file_reader.hpp/.cpp` (primary format)
   - `xml_file_reader.hpp/.cpp` (if needed)

3. Create writer interface and implementations similarly

4. Write tests

**Deliverables:**

- IFileReader and IFileWriter interfaces
- Concrete implementations
- Unit tests

**Validation:**

- [ ] Read/write round-trip preserves data
- [ ] Invalid files handled gracefully
- [ ] Tests pass

#### Task 3.3: Implement Logging System

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Create sink interface:

   ```cpp
   // src/infrastructure/logging/log_sink.hpp
   class ILogSink {
   public:
       virtual ~ILogSink() = default;
       virtual void write(LogLevel level, const std::string& message) = 0;
   };
   ```

2. Implement sinks:
   - `console_sink.hpp/.cpp`
   - `file_sink.hpp/.cpp`
   - `syslog_sink.hpp/.cpp` (Linux syslog)

3. Update Logger class:

   ```cpp
   // src/infrastructure/logging/logger.hpp
   class Logger {
       std::vector<std::unique_ptr<ILogSink>> sinks_;
       LogLevel minimumLevel_;
       std::mutex mutex_;  // Thread-safety
   public:
       static Logger& getInstance();
       void log(LogLevel level, const std::string& message);
       void addSink(std::unique_ptr<ILogSink> sink);
       void setMinimumLevel(LogLevel level);
       // Convenience methods...
   };
   ```

4. Write tests

**Deliverables:**

- Enhanced Logger with sink pattern
- 3 sink implementations
- Unit tests

**Validation:**

- [ ] Logger writes to all sinks
- [ ] Log levels filter correctly
- [ ] Thread-safe
- [ ] Tests pass

#### Task 3.4: Implement Configuration Manager

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Create `src/infrastructure/config/config_manager.hpp`:

   ```cpp
   class ConfigManager {
   public:
       static ConfigManager& getInstance();

       bool loadFromFile(const std::filesystem::path& path);
       bool loadFromEnvironment();

       template<typename T>
       T get(const std::string& key, T defaultValue = T{});

       AnalysisOptions getAnalysisOptions();
       LogLevel getLogLevel();
   };
   ```

2. Implement JSON config parser

3. Create default config: `config/default.json`

4. Write tests

**Deliverables:**

- ConfigManager class
- Config parser
- Default config file
- Unit tests

**Validation:**

- [ ] Config loads from file
- [ ] Environment variables override file config
- [ ] Default values work
- [ ] Tests pass

### 5.2 Phase 3 Summary

**Total Effort:** 30 hours

**Deliverables:**

- ✅ Strategy pattern for exporters (5 classes)
- ✅ File I/O services (readers, writers)
- ✅ Enhanced logging system (sink pattern)
- ✅ Configuration manager

**Validation Criteria:**

- [ ] All exporters produce correct output
- [ ] File I/O preserves data integrity
- [ ] Logging works with multiple sinks
- [ ] Configuration loads correctly
- [ ] All tests pass

**Risk Mitigation:**

- Test each exporter independently
- Validate round-trip I/O
- Start with simple config, extend gradually

---

## 6. Phase 4: Interface & Application Layers

**Objective:** Create facade layer and update CLI/GUI

**Duration:** 1-2 weeks (20-30 hours)

**Risk:** Low

### 6.1 Tasks

#### Task 4.1: Create Application Facade

**Effort:** 8 hours  
**Risk:** Low

**Actions:**

1. Create `src/interface/truss_analysis_facade.hpp`:

   ```cpp
   class TrussAnalysisFacade {
       std::unique_ptr<AnalysisOrchestrator> orchestrator_;
       std::unique_ptr<TrussValidator> validator_;
   public:
       AnalysisResults analyzeFromFile(const std::filesystem::path& path);
       AnalysisResults analyzeInteractive(TrussBuilder& builder);
       bool exportResults(const AnalysisResults& results, ExportFormat format,
                         const std::filesystem::path& path);
       ValidationResult validateTruss(const Truss& truss);
   };
   ```

2. Implement facade methods (workflow orchestration)

3. Write integration tests

**Deliverables:**

- TrussAnalysisFacade class
- Integration tests

**Validation:**

- [ ] Facade simplifies complex workflows
- [ ] Tests pass

#### Task 4.2: Create Truss Builder

**Effort:** 4 hours  
**Risk:** Low

**Actions:**

1. Create `src/interface/truss_builder.hpp`:

   ```cpp
   class TrussBuilder {
       std::string name_;
       std::vector<NodeData> nodes_;
       std::vector<MemberData> members_;
   public:
       TrussBuilder& setName(const std::string& name);
       TrussBuilder& addNode(double x, double y, SupportType support);
       TrussBuilder& addMember(NodeId start, NodeId end, /* properties */);
       TrussBuilder& applyForce(NodeId node, Force force);
       Truss build();
   };
   ```

2. Implement builder

3. Write tests

**Deliverables:**

- TrussBuilder class
- Unit tests

**Validation:**

- [ ] Builder creates valid trusses
- [ ] Fluent interface works
- [ ] Tests pass

#### Task 4.3: Update CLI Application

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Move `src/main_app.cpp` → `src/cli/main.cpp`
2. Create argument parser: `src/cli/argument_parser.*`
3. Create CLI controller: `src/cli/cli_controller.*`
4. Update CLI to use facade
5. Add support for:
   - Input file specification
   - Output format selection
   - Configuration overrides
   - Verbose mode

**Deliverables:**

- Refactored CLI application
- Argument parser
- CLI controller

**Validation:**

- [ ] CLI builds and runs
- [ ] All command-line arguments work
- [ ] Help text is clear

#### Task 4.4: Update GUI Application

**Effort:** 8 hours  
**Risk:** Medium

**Actions:**

1. Organize GUI files: `src/gui/widgets/`, `src/gui/project_manager/`
2. Update GUI to use facade
3. Remove any direct calls to old classes
4. Test all GUI functionality

**Deliverables:**

- Organized GUI code
- GUI uses facade

**Validation:**

- [ ] GUI builds and runs
- [ ] All features work
- [ ] No regressions

#### Task 4.5: Delete Application Singleton

**Effort:** 2 hours  
**Risk:** Low

**Actions:**

1. Delete `src/core/Application.*`
2. Move version info to CMake configuration
3. Update references

**Deliverables:**

- Application singleton removed

**Validation:**

- [ ] No references to Application class
- [ ] Version info accessible

### 6.2 Phase 4 Summary

**Total Effort:** 28 hours

**Deliverables:**

- ✅ Application facade
- ✅ Truss builder
- ✅ Updated CLI
- ✅ Updated GUI
- ✅ Application singleton removed

**Validation Criteria:**

- [ ] CLI and GUI work correctly
- [ ] Facade simplifies application code
- [ ] All features functional
- [ ] Tests pass

---

## 7. Phase 5: Build System & Deployment

**Objective:** Professional build system, Docker, and packaging

**Duration:** 1 week (20-25 hours)

**Risk:** Medium

### 7.1 Tasks

#### Task 5.1: Refactor CMake Structure

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Organize CMakeLists.txt into logical sections
2. Create CMake modules in `cmake/`:
   - `CompilerWarnings.cmake`
   - `StaticAnalysis.cmake`
   - `Sanitizers.cmake`
   - `InstallRules.cmake`

3. Define library targets properly:

   ```cmake
   add_library(TrussUtilities STATIC ...)
   add_library(TrussInfrastructure STATIC ...)
   add_library(TrussCore STATIC ...)
   add_library(TrussInterface STATIC ...)
   ```

4. Set up target dependencies

5. Configure installation rules

**Deliverables:**

- Organized CMakeLists.txt
- CMake modules
- Proper target hierarchy

**Validation:**

- [ ] Project builds successfully
- [ ] All targets link correctly
- [ ] Install target works

#### Task 5.2: Create Professional Makefile

**Effort:** 4 hours  
**Risk:** Low

**Actions:**

1. Create `Makefile` with targets:

   ```makefile
   .PHONY: all build test clean install docker-build docker-run lint format

   all: build

   build:
       cmake -B build -DCMAKE_BUILD_TYPE=Release
       cmake --build build -j$(nproc)

   test:
       cd build && ctest --output-on-failure

   clean:
       rm -rf build install

   install:
       cmake --install build --prefix $(PREFIX)

   docker-build:
       docker build -t truss-analysis:latest -f docker/Dockerfile .

   docker-run:
       docker run --rm -it truss-analysis:latest

   lint:
       find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-tidy

   format:
       find src include -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
   ```

**Deliverables:**

- Professional Makefile

**Validation:**

- [ ] All targets work
- [ ] Makefile idempotent

#### Task 5.3: Optimize Dockerfile

**Effort:** 6 hours  
**Risk:** Medium

**Actions:**

1. Create multi-stage Dockerfile:

   ```dockerfile
   # Builder stage
   FROM ubuntu:22.04 AS builder
   RUN apt-get update && apt-get install -y \
       build-essential cmake libeigen3-dev
   WORKDIR /build
   COPY . .
   RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
       && cmake --build build -j$(nproc)

   # Runtime stage (minimal)
   FROM ubuntu:22.04 AS runtime
   RUN apt-get update && apt-get install -y libeigen3-dev \
       && rm -rf /var/lib/apt/lists/*
   COPY --from=builder /build/build/TrussAnalyze /usr/local/bin/
   RUN useradd -m truss && chown truss:truss /usr/local/bin/TrussAnalyze
   USER truss
   ENTRYPOINT ["/usr/local/bin/TrussAnalyze"]
   ```

2. Create `.dockerignore`

3. Optimize layer caching

**Deliverables:**

- Optimized Dockerfile
- `.dockerignore`

**Validation:**

- [ ] Docker build succeeds
- [ ] Image size < 500MB
- [ ] Container runs correctly

#### Task 5.4: Create Packaging Scripts

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Create `scripts/package/create-deb.sh`:
   - Debian package for Ubuntu/Debian

2. Create `scripts/package/create-rpm.sh`:
   - RPM package for Fedora/RHEL

3. Create `scripts/package/create-appimage.sh`:
   - AppImage for portable deployment

**Deliverables:**

- Packaging scripts for 3 formats

**Validation:**

- [ ] DEB package installs correctly
- [ ] RPM package installs correctly
- [ ] AppImage runs on different distros

### 7.2 Phase 5 Summary

**Total Effort:** 22 hours

**Deliverables:**

- ✅ Refactored CMake build system
- ✅ Professional Makefile
- ✅ Optimized Docker container
- ✅ Packaging scripts (DEB, RPM, AppImage)

**Validation Criteria:**

- [ ] Build system clean and efficient
- [ ] Docker image < 500MB
- [ ] Packages install correctly
- [ ] All targets work

---

## 8. Phase 6: Documentation & Polish

**Objective:** Comprehensive documentation and final polish

**Duration:** 1-2 weeks (25-35 hours)

**Risk:** Low

### 8.1 Tasks

#### Task 6.1: Update Root Documentation

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Rewrite `README.md`:
   - Remove macOS references
   - Add Linux installation instructions
   - Update architecture section
   - Add badges (build status, license, version)
   - Update version to 3.0.0

2. Update `CONTRIBUTING.md`:
   - Add Linux-specific guidelines
   - Reference new directory structure
   - Update build instructions

3. Create `CHANGELOG.md`:
   - Document v3.0.0 changes
   - Migration guide from v2.x

**Deliverables:**

- Updated README.md
- Updated CONTRIBUTING.md
- New CHANGELOG.md

**Validation:**

- [ ] README clear and accurate
- [ ] All links work
- [ ] Build instructions correct

#### Task 6.2: Create Developer Documentation

**Effort:** 8 hours  
**Risk:** Low

**Actions:**

1. `docs/development/setup-guide.md`:
   - Ubuntu/Debian setup
   - Fedora setup
   - Arch Linux setup

2. `docs/development/build-instructions.md`:
   - CMake configuration options
   - Build targets
   - Troubleshooting

3. `docs/development/testing-guide.md`:
   - Test organization
   - Running tests
   - Writing tests

4. `docs/development/coding-standards.md`:
   - C++ style guide
   - Naming conventions
   - Documentation requirements

**Deliverables:**

- 4 developer guide documents

**Validation:**

- [ ] Guides are clear and complete
- [ ] Instructions work on real systems

#### Task 6.3: Create User Documentation

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. `docs/user/installation.md`:
   - Package installation
   - Building from source

2. `docs/user/quick-start.md`:
   - First analysis
   - CLI examples
   - GUI walkthrough

3. `docs/user/user-manual.md`:
   - Complete feature documentation

4. `docs/user/cli-reference.md`:
   - All command-line options
   - Examples

**Deliverables:**

- 4 user documentation files

**Validation:**

- [ ] User can follow quick start successfully
- [ ] All features documented

#### Task 6.4: Generate API Documentation

**Effort:** 4 hours  
**Risk:** Low

**Actions:**

1. Configure Doxygen (`Doxyfile`):
   - Input directories
   - Output format (HTML)
   - Styling

2. Add Doxygen comments to public API headers

3. Generate documentation

4. Create `docs/api/README.md` with links

**Deliverables:**

- Configured Doxygen
- Generated API documentation

**Validation:**

- [ ] API docs generated successfully
- [ ] All public interfaces documented

#### Task 6.5: Create Architecture Documentation

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. `docs/architecture/architecture-overview.md`:
   - Layer descriptions
   - Component diagrams (text-based)

2. `docs/architecture/data-flow.md`:
   - Analysis workflow
   - Data transformations

3. Create ADRs (Architecture Decision Records):
   - ADR-001 through ADR-006
   - Template for future ADRs

**Deliverables:**

- Architecture documentation
- Initial ADRs

**Validation:**

- [ ] Architecture clearly explained
- [ ] Diagrams helpful

#### Task 6.6: Final Code Polish

**Effort:** 6 hours  
**Risk:** Low

**Actions:**

1. Run clang-format on all code
2. Run clang-tidy and fix warnings
3. Review and improve comments
4. Remove any remaining TODOs
5. Consistent naming conventions

**Deliverables:**

- Polished, formatted code

**Validation:**

- [ ] All code formatted consistently
- [ ] No warnings from static analysis
- [ ] Code review passes

### 8.2 Phase 6 Summary

**Total Effort:** 36 hours

**Deliverables:**

- ✅ Updated root documentation
- ✅ Complete developer guides
- ✅ Complete user documentation
- ✅ Generated API documentation
- ✅ Architecture documentation
- ✅ Polished code

**Validation Criteria:**

- [ ] All documentation complete and accurate
- [ ] API docs generated
- [ ] Code formatted and clean
- [ ] No TODOs or placeholders

---

## 9. Overall Project Summary

### 9.1 Total Effort Breakdown

| Phase                            | Hours         | % of Total |
| -------------------------------- | ------------- | ---------- |
| Phase 0: Foundation              | 11            | 6%         |
| Phase 1: Test Infrastructure     | 33            | 16%        |
| Phase 2: Core Refactoring        | 48            | 23%        |
| Phase 3: Infrastructure          | 30            | 14%        |
| Phase 4: Interface & Application | 28            | 13%        |
| Phase 5: Build & Deployment      | 22            | 11%        |
| Phase 6: Documentation           | 36            | 17%        |
| **TOTAL**                        | **208 hours** | **100%**   |

### 9.2 Timeline Estimates

**Part-Time (20 hrs/week):**

- Duration: 10.4 weeks (~11 weeks)
- Calendar time: ~3 months

**Full-Time (40 hrs/week):**

- Duration: 5.2 weeks (~5 weeks)
- Calendar time: ~5-6 weeks (with buffer)

### 9.3 Risk Summary

| Risk Level | Phases     | Mitigation                                                         |
| ---------- | ---------- | ------------------------------------------------------------------ |
| **Low**    | 0, 6       | Straightforward tasks, minimal code changes                        |
| **Medium** | 1, 3, 4, 5 | Incremental approach, extensive testing                            |
| **High**   | 2          | Computational correctness validation, keep old code until verified |

### 9.4 Final Deliverables Checklist

#### Architecture & Code Quality

- [ ] Clean, layered architecture
- [ ] No monolithic classes
- [ ] SOLID principles applied
- [ ] Design patterns used appropriately
- [ ] All code formatted and linted

#### Testing

- [ ] Google Test framework
- [ ] 80%+ test coverage
- [ ] Unit, integration, system tests
- [ ] Mock objects for isolation
- [ ] Performance benchmarks

#### Build System

- [ ] Clean CMake configuration
- [ ] Professional Makefile
- [ ] Optimized Docker container
- [ ] Packaging scripts (DEB, RPM, AppImage)

#### Documentation

- [ ] Updated README
- [ ] Developer guides
- [ ] User manual
- [ ] API documentation
- [ ] Architecture documentation
- [ ] Work logs

#### Platform & Cleanup

- [ ] Linux-only codebase
- [ ] Zero macOS references
- [ ] Clean Git repository
- [ ] No build artifacts tracked

### 9.5 Success Metrics

**Quantitative:**

- Test coverage: ≥ 80%
- Docker image size: < 500MB
- Build time: < 5 minutes (clean build)
- Root directory files: < 10
- Lines of code: ~20,000 (organized)

**Qualitative:**

- Professional code organization
- Clear architectural boundaries
- Comprehensive documentation
- Easy to onboard new developers
- Portfolio-quality presentation

---

## 10. Ongoing Maintenance After Refactoring

### 10.1 Post-Refactoring Tasks

1. **CI/CD Setup:**
   - GitHub Actions workflows
   - Automated testing
   - Docker image publishing
   - Package distribution

2. **Performance Optimization:**
   - Profile critical paths
   - Optimize hot loops
   - Parallel processing (OpenMP)

3. **Feature Enhancements:**
   - Additional export formats
   - Advanced analysis options
   - Plugin system (future)

### 10.2 Continuous Improvement

1. **Regular Code Reviews**
2. **Test Coverage Monitoring**
3. **Performance Regression Testing**
4. **Documentation Updates**
5. **User Feedback Integration**

---

## 11. Conclusion

This refactoring master plan provides a comprehensive, phased approach to transforming the 2D Truss Analysis project from a practice-grade application into a production-quality, portfolio-ready tool. The plan prioritizes:

✅ **Risk Management:** Incremental changes with continuous validation  
✅ **Quality:** Comprehensive testing at every phase  
✅ **Professionalism:** Industry-standard practices throughout  
✅ **Maintainability:** Clean architecture and documentation  
✅ **Linux Focus:** Platform-specific optimization

**Estimated Timeline:** 10-12 weeks part-time, or 3-4 weeks full-time  
**Estimated Effort:** 200+ hours  
**Expected Outcome:** Portfolio-quality engineering artifact

**Next Steps:**

1. Review and approve this master plan
2. Begin Phase 0: Foundation & Cleanup
3. Track progress in work logs
4. Adjust timeline as needed based on actual progress

---

**Plan Prepared:** February 4, 2026  
**Status:** Ready for Implementation  
**Approval Required:** Yes
