# Proposed Architecture Design: 2D Truss Analysis (Refactored)

**Date:** February 4, 2026  
**Project:** 2D Truss Analysis C++ (Linux-Only)  
**Version Target:** 3.0.0

---

## 1. Architectural Overview

### 1.1 Design Philosophy

The refactored architecture follows these core principles:

1. **Separation of Concerns** - Clear boundaries between computational, data, I/O, and presentation layers
2. **SOLID Principles** - Each module has a single, well-defined responsibility
3. **Dependency Inversion** - High-level modules depend on abstractions, not implementations
4. **Testability** - All components designed for easy unit and integration testing
5. **Extensibility** - New features can be added without modifying existing code
6. **Linux-First** - No platform abstractions, optimized for Linux deployment

### 1.2 High-Level Architecture

```
┌───────────────────────────────────────────────────────────────┐
│                     APPLICATION LAYER                         │
│  ┌──────────────────┐               ┌───────────────────┐     │
│  │   CLI Frontend   │               │   GUI Frontend    │     │
│  │  (TrussAnalyze)  │               │ (TrussAnalyzeGUI) │     │
│  └────────┬─────────┘               └─────────┬─────────┘     │
│           │                                   │               │
└───────────┼───────────────────────────────────┼───────────────┘
            │                                   │
┌───────────┴───────────────────────────────────┴───────────────┐
│                     INTERFACE LAYER                           │
│  ┌──────────────────────────────────────────────────────┐     │
│  │             Application Facade                       │     │
│  │  (Simplified interface for analysis workflows)       │     │
│  └──────────────────────────────────────────────────────┘     │
└─────────────────────────────┬─────────────────────────────────┘
                              │
┌─────────────────────────────┴─────────────────────────────────┐
│                    DOMAIN LAYER (Core)                        │
│  ┌─────────────────┐  ┌─────────────┐  ┌──────────────────┐   │
│  │    Model        │  │  Analysis   │  │   Validation     │   │
│  │   (Truss        │  │   Engine    │  │    Services      │   │
│  │ Representation) │  │             │  │                  │   │
│  └─────────────────┘  └─────────────┘  └──────────────────┘   │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │           Computational Services                        │  │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐     │  │
│  │  │ Stiffness    │ │  Boundary    │ │   Linear     │     │  │
│  │  │ Assembler    │ │  Condition   │ │   Solver     │     │  │
│  │  │              │ │  Handler     │ │   Strategy   │     │  │
│  │  └──────────────┘ └──────────────┘ └──────────────┘     │  │
│  └─────────────────────────────────────────────────────────┘  │
└───────────────────────────┬───────────────────────────────────┘
                            │
┌───────────────────────────┴───────────────────────────────────┐
│                     INFRASTRUCTURE LAYER                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐     │
│  │   File I/O   │  │  Results     │  │    Logging       │     │
│  │   Services   │  │  Exporters   │  │    System        │     │
│  └──────────────┘  └──────────────┘  └──────────────────┘     │
│                                                               │
│  ┌──────────────────────────────────────────────────────┐     │
│  │           Configuration Management                   │     │
│  └──────────────────────────────────────────────────────┘     │
└───────────────────────────────────────────────────────────────┘
                            │
┌───────────────────────────┴───────────────────────────────────┐
│                    UTILITY LAYER                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐     │
│  │   Math       │  │  String      │  │    Memory        │     │
│  │   Utilities  │  │  Utilities   │  │    Utilities     │     │
│  └──────────────┘  └──────────────┘  └──────────────────┘     │
└───────────────────────────────────────────────────────────────┘
```

---

## 2. Layer Descriptions

### 2.1 Application Layer

**Responsibility:** User-facing interfaces (CLI and GUI)

**Components:**

- **CLI Frontend** (`TrussAnalyze`)
  - Command-line argument parsing
  - Interactive mode support
  - Script-friendly output
  - Return codes for automation

- **GUI Frontend** (`TrussAnalyzeGUI`)
  - Qt6-based graphical interface
  - Interactive drawing widget
  - Visualization components
  - Project file management

**Design Principles:**

- Thin layer - minimal business logic
- Delegates all computation to Domain Layer
- Handles only presentation concerns
- Separate executables for different interfaces

**Dependencies:**

- Interface Layer (Application Facade)
- Infrastructure Layer (Configuration)

---

### 2.2 Interface Layer (Application Facade)

**Responsibility:** Simplified, coarse-grained API for application workflows

**Components:**

- **TrussAnalysisFacade**
  ```cpp
  class TrussAnalysisFacade {
  public:
      // High-level workflow methods
      AnalysisResults analyzeFromFile(const std::filesystem::path& path);
      AnalysisResults analyzeInteractive(TrussBuilder& builder);
      bool exportResults(const AnalysisResults& results, ExportFormat format);
      ValidationReport validateTruss(const Truss& truss);
  };
  ```

**Design Principles:**

- Hides complexity of domain layer
- Orchestrates workflows across multiple services
- Transaction-like behavior (all-or-nothing)
- Clear error propagation

**Benefit:** Decouples application layer from domain complexity

---

### 2.3 Domain Layer (Core Computational Logic)

The heart of the application containing all structural analysis logic.

**IMPLEMENTATION STATUS (Updated February 9, 2026):**

- ✅ **Model Sublayer: COMPLETE** (Node, Member, Truss, Load entities)
- ✅ **Analysis Sublayer: COMPLETE** (AnalysisOrchestrator, StiffnessAssembler, BoundaryConditionHandler, Linear Solvers)
- ✅ **Validation Sublayer: COMPLETE** (TrussValidator with 8 validation categories)
- ⏳ **Interface Layer: NOT IMPLEMENTED** (Facades deferred to Phase 4+)

See [DOMAIN_LAYER_COMPLETION.md](../archive/DOMAIN_LAYER_COMPLETION.md) for full implementation details.

---

#### 2.3.1 Model Sublayer

**Purpose:** Represents the truss structural system

**ACTUAL IMPLEMENTATION (as of February 9, 2026):**

The Model Sublayer has been fully implemented with the following components:

1. **Value Objects** (in `src/core/Types.hpp`)

   ```cpp
   // Actual implementation uses:
   using Real = double;
   using Point2D = std::pair<Real, Real>;
   using Force2D = std::pair<Real, Real>;
   using Displacement2D = std::pair<Real, Real>;

   // IDs
   using NodeId = std::size_t;
   using MemberId = std::size_t;
   using LoadId = std::size_t;
   ```

2. **Entities** (Fully Implemented)

   ```cpp
   // src/core/model/node.hpp
   class Node {
       NodeId m_id;
       Real m_x, m_y;
       SupportType m_supportType;
   public:
       // Implemented behavior methods
       bool isConstrained() const;
       int getDegreesOfFreedom() const;
       std::vector<int> getGlobalDOFs() const;
       // ... full implementation exists
   };

   // src/core/model/member.hpp
   class Member {
       MemberId m_id;
       std::shared_ptr<Node> m_startNode, m_endNode;
       MaterialProperties m_material;
       SectionProperties m_section;
   public:
       // Implemented methods
       Real getLength() const;
       Real getAxialStiffness() const;
       bool hasNode(NodeId id) const;
       bool connectsNodes(NodeId id1, NodeId id2) const;
       // ... full implementation exists
   };

   // src/core/model/load.hpp ✅ NEW
   class Load {
       LoadId m_id;
       NodeId m_nodeId;
       LoadType m_type;
       Force2D m_force;
       std::string m_label;
   public:
       Load(LoadId id, NodeId nodeId, const Force2D& force, const std::string& label = "");
       Load(LoadId id, NodeId nodeId, Real fx, Real fy, const std::string& label = "");

       bool isZero(Real tolerance = Constants::FORCE_TOLERANCE) const;
       bool isHorizontal(Real tolerance = Constants::FORCE_TOLERANCE) const;
       bool isVertical(Real tolerance = Constants::FORCE_TOLERANCE) const;
       Real getMagnitude() const;
       // ... 12 unit tests verify all behavior
   };
   ```

3. **Aggregates** (Fully Implemented)

   ```cpp
   // src/core/model/truss.hpp
   class Truss {
       std::vector<std::shared_ptr<Node>> m_nodes;
       std::vector<std::shared_ptr<Member>> m_members;
       std::vector<Load> m_loads;
   public:
       // Aggregate root responsibilities (IMPLEMENTED)
       std::shared_ptr<Node> addNode(Real x, Real y, SupportType support = SupportType::Free);
       std::shared_ptr<Member> addMember(std::shared_ptr<Node> start, std::shared_ptr<Node> end,
                                         const MaterialProperties& material,
                                         const SectionProperties& section);
       void addLoad(const Load& load);

       // Query methods (IMPLEMENTED)
       const std::vector<std::shared_ptr<Node>>& getNodes() const;
       const std::vector<std::shared_ptr<Member>>& getMembers() const;
       const std::vector<Load>& getLoads() const;
       std::vector<std::shared_ptr<Member>> getMembersConnectedTo(NodeId nodeId) const;
       std::vector<std::shared_ptr<Member>> getMembersAtNode(const std::shared_ptr<Node>& node) const;
       std::vector<std::shared_ptr<Node>> getConstrainedNodes() const;
       std::vector<std::shared_ptr<Node>> getLoadedNodes() const;
       std::vector<std::shared_ptr<Node>> getFreeNodes() const;

       size_t getNodeCount() const;
       size_t getMemberCount() const;
       size_t getConstrainedDofs() const;
       // ... 12 unit tests verify aggregate behavior
   };
   ```

**Design Principles (VALIDATED):**

- ✅ Rich domain model with behavior, not anemic data holders
- ✅ Entities have identity and lifecycle
- ✅ Aggregate root (Truss) controls all modifications
- ✅ Framework-independent (no Qt/GUI dependencies)

#### 2.3.2 Analysis Sublayer

**Purpose:** Structural analysis computations

**ACTUAL IMPLEMENTATION (as of February 9, 2026):**

The Analysis Sublayer has been fully decomposed from the original monolithic AnalysisEngine:

```cpp
// src/core/analysis/analysis_orchestrator.hpp (IMPLEMENTED)
class AnalysisOrchestrator {
    std::unique_ptr<StiffnessAssembler> m_stiffnessAssembler;
    std::unique_ptr<BoundaryConditionHandler> m_bcHandler;
    std::unique_ptr<ILinearSolver> m_solver;
public:
    AnalysisResults analyze(const Truss& truss);
    // ... 10 unit tests verify orchestration
};

// src/core/analysis/stiffness_assembler.hpp (IMPLEMENTED)
class StiffnessAssembler {
public:
    Eigen::SparseMatrix<Real> assembleGlobalStiffness(const Truss& truss);
private:
    Eigen::Matrix4d computeLocalStiffness(const Member& member);
    Eigen::Matrix4d computeTransformationMatrix(const Member& member);
    // ... 4 unit tests verify assembly correctness
};

// src/core/analysis/boundary_condition_handler.hpp (IMPLEMENTED)
class BoundaryConditionHandler {
public:
    struct ConstrainedSystem {
        Eigen::SparseMatrix<Real> Kff;
        Eigen::VectorXd Ff;
        std::vector<int> constrainedDOFs;
    };

    ConstrainedSystem applyBoundaryConditions(
        const Eigen::SparseMatrix<Real>& K,
        const Eigen::VectorXd& F,
        const Truss& truss
    );
    // ... 10 unit tests verify constraint application
};

// src/core/analysis/linear_solver.hpp (IMPLEMENTED)
class ILinearSolver {  // Interface
public:
    virtual ~ILinearSolver() = default;
    virtual Eigen::VectorXd solve(
        const Eigen::SparseMatrix<Real>& A,
        const Eigen::VectorXd& b
    ) = 0;
};

class DirectSolver : public ILinearSolver { /* ... */ };
class IterativeSolver : public ILinearSolver { /* ... */ };
// ... 17 unit tests verify both solvers
```

**Benefits (ACHIEVED):**

- ✅ Each class has single responsibility
- ✅ Testable in isolation (72+ analysis tests passing)
- ✅ Solver strategy can be swapped at runtime
- ✅ Legacy AnalysisEngine fully removed (412 lines deleted)

#### 2.3.3 Validation Sublayer

**Purpose:** Structural integrity and input validation

**ACTUAL IMPLEMENTATION (as of February 9, 2026):**

The Validation Sublayer has been fully implemented with comprehensive validation services:

```cpp
// src/core/validation/TrussValidator.hpp (IMPLEMENTED ✅)
enum class ValidationSeverity {
    Info,     // Informational message
    Warning,  // Potential issue but analysis may proceed
    Error,    // Critical issue preventing valid analysis
    Fatal     // Structural impossibility or data corruption
};

struct ValidationIssue {
    ValidationSeverity severity;
    std::string category;        // "Geometry", "Boundary", "Material", etc.
    std::string message;         // Human-readable description
    std::string technicalDetail; // Technical explanation for engineers
    std::vector<NodeId> affectedNodes;
    std::vector<MemberId> affectedMembers;
};

class ValidationResult {
    std::vector<ValidationIssue> m_issues;
public:
    bool isValid() const;  // No errors or fatal issues
    bool hasErrors() const;
    bool hasFatal() const;
    std::vector<ValidationIssue> getIssuesBySeverity(ValidationSeverity) const;
    std::vector<ValidationIssue> getIssuesByCategory(const std::string&) const;
    std::string getSummary() const;
    // ... 2 unit tests verify filtering and summary
};

class TrussValidator {
public:
    ValidationResult validate(const Truss& truss) const;
    bool isValid(const Truss& truss) const;  // Quick check

    // Individual validation categories (ALL IMPLEMENTED ✅)
    void validateStructuralCompleteness(const Truss&, ValidationResult&) const;
    void validateGeometry(const Truss&, ValidationResult&) const;
    void validateMaterials(const Truss&, ValidationResult&) const;
    void validateBoundaryConditions(const Truss&, ValidationResult&) const;
    void validateStaticDeterminacy(const Truss&, ValidationResult&) const;  // ⚠️ Bug fixed
    void validateKinematicStability(const Truss&, ValidationResult&) const;
    void validateLoads(const Truss&, ValidationResult&) const;
    void validateConnectivity(const Truss&, ValidationResult&) const;
    // ... 25+ unit tests verify all 8 categories
};
```

**Validation Rules (IMPLEMENTED AND TESTED):**

- ✅ **Structural Completeness:** Minimum 2 nodes, 1 member; no null pointers
- ✅ **Geometry:** Zero-length members, coincident nodes, duplicates, NaN/infinity checks
- ✅ **Materials:** Positive Young's modulus, area, density, yield strength
- ✅ **Boundary Conditions:** Minimum 3 constraints, adequate support
- ✅ **Static Determinacy:** Formula 2n = m + r (correctly implemented after bug fix)
- ✅ **Kinematic Stability:** Minimum constraints, isolated node detection
- ✅ **Loads:** Force application validation, NaN detection, constrained node warnings
- ✅ **Connectivity:** No self-loops, valid node references

**CRITICAL BUG FIX (February 9, 2026):**

- **Issue:** Static determinacy logic was inverted (indeterminate ↔ unstable swapped)
- **Fix:** Corrected conditional: `determinacyCheck < 0 → indeterminate`, `> 0 → unstable`
- **Validation:** Hand-calculated test cases confirm correctness
- **Details:** See [DOMAIN_LAYER_COMPLETION.md](../archive/DOMAIN_LAYER_COMPLETION.md) section 5

**Test Coverage:**

- 25+ unit tests covering all 8 validation categories
- 100% coverage of validation logic
- Engineering rules validated with hand-calculated test cases

---

### 2.4 Infrastructure Layer

**Responsibility:** Technical services supporting domain layer

#### 2.4.1 File I/O Services ✅ **IMPLEMENTED (February 13, 2026)**

```cpp
// src/infrastructure/io/truss_reader.hpp
class ITrussReader {  // Interface
public:
    virtual ~ITrussReader() = default;
    virtual std::shared_ptr<core::Truss> read(
        const std::filesystem::path& filepath,
        const FileIOOptions& options
    ) = 0;
    virtual bool supportsFormat(FileFormat format) const = 0;
    virtual FileFormat getFormat() const = 0;
};

class JsonTrussReader : public ITrussReader { /* Implemented */ };
class XmlTrussReader : public ITrussReader { /* Implemented */ };

// src/infrastructure/io/truss_writer.hpp
class ITrussWriter {
public:
    virtual ~ITrussWriter() = default;
    virtual bool write(
        const core::Truss& truss,
        const std::filesystem::path& filepath,
        const FileIOOptions& options
    ) = 0;
    virtual bool supportsFormat(FileFormat format) const = 0;
    virtual FileFormat getFormat() const = 0;
};

class JsonTrussWriter : public ITrussWriter { /* Implemented */ };
class XmlTrussWriter : public ITrussWriter { /* Implemented */ };

// src/infrastructure/io/fileio_factory.hpp
class FileIOFactory {
public:
    static std::unique_ptr<ITrussReader> createReader(FileFormat format);
    static std::unique_ptr<ITrussWriter> createWriter(FileFormat format);
    static FileFormat detectFormat(const std::filesystem::path& path);
};
```

**Status:** ✅ Complete (36 tests, 100% pass rate)  
**Design:** Strategy pattern with Factory for multiple file formats  
**Features:**

- Strict referential integrity validation (duplicate IDs, unknown references)
- Node ID mapping supporting non-sequential IDs
- Clear exception hierarchy (ParseException vs ValidationException)
- JSON and XML format support
- Extensible for future formats (CSV, YAML)

**Documentation:** [Work Log](../work-logs/2026-02-13-file-io-referential-integrity-implementation.md)

#### 2.4.2 Results Exporters ✅ **IMPLEMENTED (February 8, 2026)**

**Current Problem:** Single class handles all export formats

**Solution:** Strategy pattern with separate exporters

```cpp
// src/infrastructure/export/exporter.hpp
class IResultsExporter {  // Interface
public:
    virtual ~IResultsExporter() = default;
    virtual bool exportResults(
        const AnalysisResults& results,
        const std::filesystem::path& path
    ) = 0;
};

// Concrete implementations
class CsvExporter : public IResultsExporter { /* ... */ };
class JsonExporter : public IResultsExporter { /* ... */ };
class HtmlExporter : public IResultsExporter { /* ... */ };
class LatexExporter : public IResultsExporter { /* ... */ };

// Factory for exporter creation
class ExporterFactory {
public:
    static std::unique_ptr<IResultsExporter> create(ExportFormat format);
};
```

**Benefits:**

- Each exporter is independent
- Easy to add new formats
- Testable in isolation
- Can be parallelized

#### 2.4.3 Logging System

**Current Problem:** Simple logger lacks configuration and structure

**Solution:** Structured logging with levels and sinks

```cpp
// src/infrastructure/logging/logger.hpp
enum class LogLevel {
    Trace, Debug, Info, Warning, Error, Critical
};

class ILogSink {  // Interface
public:
    virtual ~ILogSink() = default;
    virtual void write(LogLevel level, const std::string& message) = 0;
};

class ConsoleSink : public ILogSink { /* ... */ };
class FileSink : public ILogSink { /* ... */ };
class SyslogSink : public ILogSink { /* Linux syslog */ };

class Logger {
    std::vector<std::unique_ptr<ILogSink>> sinks;
    LogLevel minimumLevel;
public:
    static Logger& getInstance();  // Singleton (acceptable for logging)

    void log(LogLevel level, const std::string& message);
    void addSink(std::unique_ptr<ILogSink> sink);
    void setMinimumLevel(LogLevel level);

    // Convenience methods
    void trace(const std::string& msg);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);
    void critical(const std::string& msg);
};
```

**Features:**

- Multiple output sinks
- Configurable log levels
- Thread-safe
- Structured logging (future: JSON format)

#### 2.4.4 Configuration Management

**New Component:** Centralized configuration

```cpp
// src/infrastructure/config/config_manager.hpp
class ConfigManager {
public:
    static ConfigManager& getInstance();

    // Load configuration
    bool loadFromFile(const std::filesystem::path& path);
    bool loadFromEnvironment();

    // Query configuration
    template<typename T>
    T get(const std::string& key, T defaultValue = T{});

    bool has(const std::string& key);

    // Analysis configuration
    AnalysisOptions getAnalysisOptions();
    LogLevel getLogLevel();
    std::filesystem::path getDataDirectory();
};
```

**Configuration Sources (priority order):**

1. Command-line arguments
2. Environment variables
3. User config file (`~/.config/truss-analysis/config.json`)
4. System config file (`/etc/truss-analysis/config.json`)
5. Default values

---

### 2.5 Utility Layer

**Responsibility:** Low-level, reusable utilities

```cpp
// src/utilities/math_utils.hpp
namespace truss::utils::math {
    bool nearlyEqual(double a, double b, double epsilon = 1e-9);
    double degreesToRadians(double degrees);
    Eigen::Matrix2d rotationMatrix(double angle);
}

// src/utilities/string_utils.hpp
namespace truss::utils::string {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string formatScientific(double value, int precision);
}

// src/utilities/error_handling.hpp
namespace truss::utils::error {
    class TrussException : public std::runtime_error {
        ErrorCode code;
    public:
        TrussException(ErrorCode code, const std::string& message);
        ErrorCode getCode() const;
    };

    // Specific exception types
    class ValidationException : public TrussException { /* ... */ };
    class AnalysisException : public TrussException { /* ... */ };
    class FileIOException : public TrussException { /* ... */ };
}
```

---

## 3. Module Dependencies

### 3.1 Dependency Graph

```
Application Layer
    ↓ depends on
Interface Layer (Facade)
    ↓ depends on
Domain Layer (Core)
    ↓ depends on
Infrastructure Layer
    ↓ depends on
Utility Layer

NOTE: Arrows only go downward - no upward dependencies
```

### 3.2 Dependency Rules

1. **No Circular Dependencies:** Strictly enforced through CMake targets
2. **Interface Segregation:** Large interfaces split into focused interfaces
3. **Dependency Injection:** Dependencies passed through constructors
4. **Abstract Interfaces:** High-level modules depend on interfaces, not implementations

### 3.3 CMake Target Structure

```cmake
# Utility library (no dependencies)
add_library(TrussUtilities STATIC
    src/utilities/*.cpp
)

# Infrastructure library (depends on Utilities)
add_library(TrussInfrastructure STATIC
    src/infrastructure/*.cpp
)
target_link_libraries(TrussInfrastructure PRIVATE TrussUtilities)

# Domain/Core library (depends on Infrastructure, Utilities)
add_library(TrussCore STATIC
    src/core/*.cpp
)
target_link_libraries(TrussCore PRIVATE
    TrussInfrastructure
    TrussUtilities
    Eigen3::Eigen
)

# Interface library (depends on Core)
add_library(TrussInterface STATIC
    src/interface/*.cpp
)
target_link_libraries(TrussInterface PRIVATE TrussCore)

# CLI application
add_executable(TrussAnalyze
    src/cli/main.cpp
)
target_link_libraries(TrussAnalyze PRIVATE TrussInterface)

# GUI application
add_executable(TrussAnalyzeGUI
    src/gui/*.cpp
)
target_link_libraries(TrussAnalyzeGUI PRIVATE
    TrussInterface
    Qt6::Core
    Qt6::Widgets
)
```

**Benefits:**

- Clear dependency hierarchy
- Parallel compilation
- Testable in isolation
- Reusable libraries

---

## 4. Design Patterns Applied

### 4.1 Creational Patterns

**Factory Pattern** - Exporter creation, Solver creation

```cpp
auto exporter = ExporterFactory::create(ExportFormat::CSV);
auto solver = SolverFactory::create(SolverType::Direct);
```

**Builder Pattern** - Truss construction

```cpp
TrussBuilder builder("Example Truss");
builder.addNode(0, 0, SupportType::Pinned)
       .addNode(4, 0, SupportType::RollerY)
       .addMember(0, 1, material, section)
       .applyForce(1, Force(0, -1000));
Truss truss = builder.build();
```

### 4.2 Structural Patterns

**Facade Pattern** - Application interface simplification

```cpp
TrussAnalysisFacade facade;
auto results = facade.analyzeFromFile("model.json");
facade.exportResults(results, ExportFormat::CSV);
```

**Strategy Pattern** - Interchangeable algorithms

- Linear solvers (Direct vs Iterative)
- File readers (JSON, XML, CSV)
- Result exporters (CSV, JSON, HTML, LaTeX)

**Adapter Pattern** - External library integration

```cpp
class EigenSolverAdapter : public ILinearSolver {
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    // Adapts Eigen's interface to our ILinearSolver
};
```

### 4.3 Behavioral Patterns

**Template Method** - Analysis workflow

```cpp
class AnalysisTemplate {
protected:
    virtual void preprocess();
    virtual void assemble();
    virtual void solve();
    virtual void postprocess();
public:
    AnalysisResults execute() {
        preprocess();
        assemble();
        solve();
        postprocess();
    }
};
```

**Observer Pattern** - GUI updates (future)

```cpp
class IAnalysisObserver {
public:
    virtual void onProgress(int percentage) = 0;
    virtual void onComplete(const AnalysisResults& results) = 0;
};
```

---

## 5. Error Handling Strategy

### 5.1 Exception Hierarchy

```cpp
namespace truss::errors {

class TrussException : public std::runtime_error {
    ErrorCode code_;
    std::string context_;
public:
    TrussException(ErrorCode code, const std::string& message);
    ErrorCode code() const;
    const std::string& context() const;
};

// Domain exceptions
class ValidationException : public TrussException { };
class AnalysisException : public TrussException { };
class SingularMatrixException : public AnalysisException { };
class ConvergenceException : public AnalysisException { };

// Infrastructure exceptions
class FileIOException : public TrussException { };
class ParseException : public FileIOException { };
class ConfigurationException : public TrussException { };

}  // namespace truss::errors
```

### 5.2 Error Codes

```cpp
enum class ErrorCode {
    // Validation errors (1000-1999)
    InvalidGeometry = 1001,
    StaticallyIndeterminate = 1002,
    UnstableStructure = 1003,
    InvalidMaterialProperties = 1004,

    // Analysis errors (2000-2999)
    SingularStiffnessMatrix = 2001,
    ConvergenceFailure = 2002,
    InsufficientConstraints = 2003,

    // I/O errors (3000-3999)
    FileNotFound = 3001,
    ParseError = 3002,
    WriteError = 3003,

    // Configuration errors (4000-4999)
    InvalidConfiguration = 4001,
    MissingRequiredParameter = 4002
};
```

### 5.3 Error Handling Guidelines

1. **Use exceptions for exceptional conditions** - Not for control flow
2. **Return std::expected<T, Error> for expected errors** (C++23 or boost)
3. **Log all errors** with context
4. **Provide actionable error messages** for users
5. **Clean up resources** using RAII

---

## 6. Threading and Concurrency

### 6.1 Current State

- Single-threaded analysis
- GUI runs on main thread

### 6.2 Future Parallelization Opportunities

**Phase 1 (Simple):**

- Parallel stiffness matrix assembly (OpenMP)
- Background analysis execution (GUI remains responsive)

**Phase 2 (Advanced):**

- Parallel solver (sparse matrix operations)
- Parallel export to multiple formats

**Design Consideration:**

- All computational classes are thread-safe (stateless or immutable)
- Shared state protected by mutexes
- Logger is thread-safe

---

## 7. Memory Management

### 7.1 Smart Pointer Strategy

```cpp
// Ownership
std::unique_ptr<T>  // Single owner
std::shared_ptr<T>  // Shared ownership (use sparingly)

// Non-owning references
T*                  // Raw pointer for non-owning reference
std::reference_wrapper<T>  // Safer alternative

// Containers
std::vector<std::unique_ptr<T>>  // Vector of owned objects
std::vector<T*>                  // Vector of references
```

### 7.2 Memory Optimization

- **Eigen3 aligned allocators** for vectorization
- **Reserve container capacity** to avoid reallocations
- **Use Eigen::Ref<>** for matrix views (avoid copies)
- **Pool allocator** for frequent small allocations (future)

---

## 8. Configuration and Extensibility

### 8.1 Plugin Architecture (Future)

While not implemented in Phase 1, the architecture supports future plugins:

```cpp
// Interface for analysis plugins
class IAnalysisPlugin {
public:
    virtual ~IAnalysisPlugin() = default;
    virtual std::string getName() const = 0;
    virtual void initialize(const Truss& truss) = 0;
    virtual void execute(AnalysisResults& results) = 0;
};

// Examples:
// - BucklingAnalysisPlugin
// - DynamicAnalysisPlugin
// - NonlinearAnalysisPlugin
```

### 8.2 Extension Points

1. **Custom Material Models** - Derive from `IMaterialModel`
2. **Custom Export Formats** - Implement `IResultsExporter`
3. **Custom Solvers** - Implement `ILinearSolver`
4. **Custom Validation Rules** - Add to `TrussValidator`

---

## 9. Performance Considerations

### 9.1 Optimization Strategy

**Computational Hotspots:**

1. Stiffness matrix assembly - **Use Eigen's sparse matrices**
2. Linear system solving - **Direct solver for small systems (<10000 DOF), iterative for large**
3. Results export - **Lazy evaluation, stream output**

**Profiling Points:**

- Matrix assembly time
- Solver time
- Export time
- Memory allocation

### 9.2 Benchmarking

```cpp
// src/core/benchmark/benchmark_suite.hpp
class BenchmarkSuite {
public:
    void runAllBenchmarks();
    void benchmarkSmallTruss();    // 10 nodes
    void benchmarkMediumTruss();   // 100 nodes
    void benchmarkLargeTruss();    // 1000 nodes
    void benchmarkExport();
};
```

---

## 10. Testability

### 10.1 Test Architecture

Each layer has corresponding test directory:

```
tests/
├── unit/                  # Unit tests for isolated components
│   ├── core/
│   ├── infrastructure/
│   └── utilities/
├── integration/           # Integration tests across layers
│   ├── analysis_workflow/
│   └── file_io/
├── system/                # End-to-end system tests
└── performance/           # Performance regression tests
```

### 10.2 Testable Design Features

1. **Dependency Injection** - Mock dependencies in tests
2. **Interface Abstractions** - Create test doubles
3. **Pure Functions** - Easy to test, no side effects
4. **Factory Functions** - Inject test implementations
5. **No Singletons** (except Logger) - Testable in isolation

### 10.3 Test Fixtures

```cpp
class TrussTestFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    // Helper methods
    Truss createSimpleTriangularTruss();
    Truss createBridgeTruss();
    MaterialProperties getSteelProperties();
};
```

---

## 11. Migration Strategy from Current Architecture

### 11.1 Incremental Refactoring

**Phase 1:** Create new architecture alongside old

- Introduce new directory structure
- Implement new classes without removing old ones
- Write comprehensive tests for new implementation

**Phase 2:** Migrate functionality

- Port analysis logic to new classes
- Update GUI to use new facade
- Maintain backward compatibility

**Phase 3:** Remove old code

- Delete deprecated classes
- Clean up temporary compatibility layer
- Update all references

### 11.2 Compatibility Layer

```cpp
// Temporary wrapper during migration
namespace legacy {
    class LegacyAnalysisEngine {
        std::unique_ptr<AnalysisOrchestrator> newImpl;
    public:
        // Old interface delegating to new implementation
        AnalysisResults analyze(const Truss& truss);
    };
}
```

---

## 12. Architecture Decision Records (ADRs)

Key decisions documented:

1. **ADR-001:** Use Strategy pattern for exporters instead of single class
2. **ADR-002:** Decompose AnalysisEngine into smaller, focused classes
3. **ADR-003:** Use Facade pattern for application interface
4. **ADR-004:** Introduce value objects (Force, Displacement) instead of raw doubles
5. **ADR-005:** Use Eigen3 sparse matrices for large systems
6. **ADR-006:** Logging system uses Sink pattern for flexibility

_(Full ADRs to be created in separate documents)_

---

## 13. Summary of Architectural Improvements

| Aspect              | Current                     | Refactored                    | Benefit                      |
| ------------------- | --------------------------- | ----------------------------- | ---------------------------- |
| **AnalysisEngine**  | Monolithic (300+ lines)     | Decomposed (4 classes)        | Testability, maintainability |
| **ResultsExporter** | Single class with 5 methods | Strategy pattern (5 classes)  | Open/Closed, extensibility   |
| **Application**     | Unclear singleton           | Removed, replaced with Facade | Clarity, testability         |
| **Value Objects**   | Raw doubles                 | Force, Displacement classes   | Type safety, expressiveness  |
| **Error Handling**  | Generic exceptions          | Typed exception hierarchy     | Specific error handling      |
| **Configuration**   | Hardcoded                   | ConfigManager with sources    | Flexibility, deployment      |
| **Logging**         | Simple output               | Structured with sinks         | Production-ready             |
| **Dependencies**    | Implicit                    | Explicit (DI, interfaces)     | Testability, flexibility     |

---

## 14. Conclusion

This refactored architecture transforms the 2D Truss Analysis application from a practice project into a professional, production-quality tool. Key improvements:

✅ **Clear separation of concerns** across 5 well-defined layers  
✅ **SOLID principles** applied consistently  
✅ **Testable design** with dependency injection and interfaces  
✅ **Extensible architecture** supporting future enhancements  
✅ **Professional patterns** (Strategy, Factory, Facade, Builder)  
✅ **Linux-optimized** without platform abstractions  
✅ **Maintainable codebase** with clear module responsibilities

The architecture supports the project's transformation into a portfolio-quality artifact demonstrating professional software engineering skills.

---

**Next:** Proceed to detailed Directory Structure design and Implementation Roadmap.
