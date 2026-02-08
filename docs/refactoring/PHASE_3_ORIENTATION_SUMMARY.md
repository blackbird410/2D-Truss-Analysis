# Phase 3 Infrastructure Layer Refactoring: Orientation Summary

**Generated:** 2026-02-07  
**Project Version:** v3.0.0  
**Prepared By:** Civil Engineering Software Solutions  
**Purpose:** Mandatory orientation review before Phase 3 implementation

---

## Executive Summary

This document provides comprehensive orientation for Phase 3 (Infrastructure Layer Refactoring) of the 2D Truss Analysis project refactoring initiative. Phase 3 aims to separate infrastructure concerns (I/O, export, logging, configuration) from the domain model following SOLID principles and design patterns.

**Current State:**

- Phase 0 (Foundation), Phase 1 (Test Infrastructure), Phase 2 (Core Domain) **COMPLETE** ✅
- Infrastructure components currently co-located with domain logic in `src/core/`
- No dedicated `src/infrastructure/` directory exists
- Zero infrastructure test coverage despite 72 passing domain tests

**Phase 3 Objectives:**

- Refactor `ResultsExporter` using Strategy pattern (10h)
- Implement File I/O Services with interface abstraction (8h)
- Enhance Logger with sink pattern (6h)
- Implement Configuration Manager (6h)
- **Total Effort:** 30 hours

---

## 1. Current Infrastructure State Analysis

### 1.1 Component Inventory

#### A. ResultsExporter (Monolithic Design)

**Location:** `src/core/ResultsExporter.{hpp,cpp}`  
**Size:** 166 lines (header) + 629 lines (implementation) = **795 lines total**  
**Responsibility:** Export analysis results to 6 formats

**Current Architecture:**

```cpp
class ResultsExporter {
public:
    bool exportResults(const Truss& truss,
                      const AnalysisResults& results,
                      const std::string& fileName,
                      ExportFormat format,
                      const ExportOptions& options);

private:
    // 6 format-specific methods (~100-150 lines each)
    bool exportToCSV(...);
    bool exportToJSON(...);
    bool exportToXML(...);
    bool exportToHTML(...);
    bool exportToLaTeX(...);
    bool exportToText(...);
};
```

**Design Issues:**

- ✗ Violates Single Responsibility Principle (SRP)
- ✗ All formats in one God Class (629 lines)
- ✗ Switch statement routing in `exportResults()` method
- ✗ Direct file I/O throughout (no abstraction)
- ✗ Tight coupling to domain (takes `Truss`, `AnalysisResults`)
- ✗ Cannot add new formats without modifying class (violates OCP)

**Current Usage:**

- **GUI:** [MainWindow.cpp:459](../src/gui/MainWindow.cpp#L459) - User export action
- **Format Detection:** Automatic extension-based detection (`.csv`, `.json`, `.xml`, etc.)
- **Options:** Comprehensive `ExportOptions` struct with 12 boolean flags + precision/delimiter

**Positive Aspects:**

- ✓ Format detection works well (`detectFormat()` helper)
- ✓ ExportOptions struct is well-designed (preserve in refactor)
- ✓ Error handling with `m_lastError` member
- ✓ Utility methods: `formatNumber()`, `formatTimestamp()`, `escapeString()`

---

#### B. Logger (Static Singleton)

**Location:** `src/core/Logger.{hpp,cpp}`  
**Size:** 112 lines (header) + 144 lines (implementation) = **256 lines total**  
**Responsibility:** Application-wide logging

**Current Architecture:**

```cpp
class Logger {
public:
    static void initialize();
    static void shutdown();
    static void setLevel(LogLevel level);

    // 6 log level methods
    static void trace(const std::string& message);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);

private:
    static std::ofstream s_logFile;
    static LogLevel s_currentLevel;
    static bool s_initialized;
    static std::mutex s_logMutex;
};
```

**Design Issues:**

- ✗ Hardcoded filename "TrussAnalysis2D.log"
- ✗ No sink abstraction (cannot write to multiple destinations)
- ✗ Static interface prevents dependency injection
- ✗ No configurability (file path, rotation policy, etc.)
- ✗ Console + file output always on (no control)

**Current Usage:**

- **Initialization:** [main.cpp](../src/main.cpp) calls `Logger::initialize()` at startup
- **Shutdown:** [main.cpp](../src/main.cpp) calls `Logger::shutdown()` at exit
- **Application:** [Application.cpp:21-25](../src/core/Application.cpp#L21-L25) - Info/warn messages during init/shutdown
- **Thread Safety:** Uses `std::mutex` (s_logMutex) for thread-safe writes

**Positive Aspects:**

- ✓ Thread-safe with mutex
- ✓ 6 log levels (trace → critical)
- ✓ Timestamp formatting
- ✓ Clean interface (simple to use)

---

#### C. File I/O (No Abstraction)

**Current State:** **NOT IMPLEMENTED** - No `IFileReader` or `IFileWriter` interfaces exist

**Current File I/O Locations:**

1. **ProjectFileManager (GUI):** [src/gui/ProjectFileManager.{hpp,cpp}](../src/gui/ProjectFileManager.cpp)
   - `saveProject()` - Qt JSON serialization (QJsonDocument)
   - `loadProject()` - Qt JSON deserialization
   - **Format:** `.truss` files (JSON structure)
   - **Coupling:** Tightly coupled to Qt framework and GUI layer
2. **ResultsExporter:** Direct `std::ofstream` calls in all 6 export methods
   - CSV export: [ResultsExporter.cpp:47-109](../src/core/ResultsExporter.cpp#L47-L109)
   - JSON export: [ResultsExporter.cpp:115-231](../src/core/ResultsExporter.cpp#L115-L231)
   - XML export: [ResultsExporter.cpp:274-316](../src/core/ResultsExporter.cpp#L274-L316)
   - **Issue:** No error handling abstraction, no retry logic

**Master Plan Target:**

```cpp
// Target: src/infrastructure/io/file_reader.hpp
class IFileReader {
public:
    virtual ~IFileReader() = default;
    virtual Truss read(const std::filesystem::path& path) = 0;
};

// Implementations:
class JsonFileReader : public IFileReader { /* ... */ };
class XmlFileReader : public IFileReader { /* ... */ };
```

---

#### D. Configuration Manager

**Current State:** **DOES NOT EXIST**

**Current Hardcoded Settings:**

- Logger filename: "TrussAnalysis2D.log" (hardcoded in [Logger.cpp:21](../src/core/Logger.cpp#L21))
- Export precision: Passed via `ExportOptions` struct (user-controlled)
- Analysis solver settings: Direct parameters to `AnalysisEngine`
- No environment variable support
- No config file support

**Master Plan Target:**

```cpp
// Target: src/infrastructure/config/config_manager.hpp
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

**Planned Config File:** `config/default.json`

---

### 1.2 Directory Structure Gap Analysis

**Current Structure:**

```
src/
├── core/
│   ├── Application.{hpp,cpp}          # Uses Logger
│   ├── Logger.{hpp,cpp}               # Infrastructure concern (misplaced)
│   ├── ResultsExporter.{hpp,cpp}      # Infrastructure concern (misplaced)
│   ├── analysis/                      # Domain - Correct location
│   │   ├── AnalysisEngine.{hpp,cpp}
│   │   ├── StiffnessAssembler.{hpp,cpp}
│   │   ├── BoundaryConditionHandler.{hpp,cpp}
│   │   └── solvers/
│   └── model/                         # Domain - Correct location
│       ├── Truss.{hpp,cpp}
│       ├── Node.{hpp,cpp}
│       └── Member.{hpp,cpp}
└── gui/
    ├── MainWindow.{hpp,cpp}           # Uses ResultsExporter, ProjectFileManager
    └── ProjectFileManager.{hpp,cpp}   # Qt-specific I/O (tightly coupled)
```

**Target Structure (Phase 3):**

```
src/
├── core/
│   ├── Application.{hpp,cpp}          # Uses Logger (via interface)
│   ├── analysis/                      # Domain - No changes
│   └── model/                         # Domain - No changes
├── infrastructure/                     # NEW DIRECTORY
│   ├── export/                        # NEW - Strategy pattern exporters
│   │   ├── exporter.hpp               # IResultsExporter interface
│   │   ├── exporter_factory.hpp
│   │   ├── exporter_factory.cpp
│   │   ├── csv_exporter.{hpp,cpp}
│   │   ├── json_exporter.{hpp,cpp}
│   │   ├── xml_exporter.{hpp,cpp}
│   │   ├── html_exporter.{hpp,cpp}
│   │   └── latex_exporter.{hpp,cpp}
│   ├── io/                            # NEW - File I/O services
│   │   ├── file_reader.hpp            # IFileReader interface
│   │   ├── file_writer.hpp            # IFileWriter interface
│   │   ├── json_file_reader.{hpp,cpp}
│   │   └── json_file_writer.{hpp,cpp}
│   ├── logging/                       # NEW - Enhanced logging
│   │   ├── logger.{hpp,cpp}           # Refactored with sinks
│   │   ├── log_sink.hpp               # ILogSink interface
│   │   ├── console_sink.{hpp,cpp}
│   │   ├── file_sink.{hpp,cpp}
│   │   └── syslog_sink.{hpp,cpp}      # Linux syslog integration
│   └── config/                        # NEW - Configuration management
│       ├── config_manager.{hpp,cpp}
│       └── config_parser.{hpp,cpp}
└── gui/
    ├── MainWindow.{hpp,cpp}           # Updated to use ExporterFactory
    └── ProjectFileManager.{hpp,cpp}   # May migrate to IFileReader/Writer
```

**Migration Actions:**

1. Create `src/infrastructure/` directory structure (4 subdirectories)
2. Move and refactor `Logger` → `src/infrastructure/logging/`
3. Decompose and move `ResultsExporter` → `src/infrastructure/export/` (5 concrete exporters)
4. Create new `src/infrastructure/io/` (File I/O services)
5. Create new `src/infrastructure/config/` (Configuration manager)

---

## 2. Coupling & Dependency Analysis

### 2.1 Infrastructure → Domain Coupling

#### ResultsExporter Dependencies (HIGH COUPLING)

```cpp
// Current tight coupling to domain
bool ResultsExporter::exportResults(
    const Truss& truss,              // Domain model dependency
    const AnalysisResults& results,   // Domain analysis dependency
    const std::string& fileName,
    ExportFormat format,
    const ExportOptions& options
);
```

**Issue:** Infrastructure layer depends on domain model (violation of Dependency Inversion Principle)

**Refactoring Strategy:**

- Introduce `IResultsExporter` interface in infrastructure layer
- Each concrete exporter receives domain data as parameters
- Use Dependency Injection to inject exporters into GUI
- Domain layer remains unaware of infrastructure

**Example Refactored Interface:**

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

---

#### Logger Dependencies (MEDIUM COUPLING)

**Current Usage Points:**

- **main.cpp:** `Logger::initialize()` / `Logger::shutdown()` (lifecycle)
- **Application.cpp:21:** `Logger::warn("Application already initialized")`
- **Application.cpp:24:** `Logger::info("Initializing application...")`
- **Application.cpp:32:** `Logger::info("Application initialization completed")`
- **Application.cpp:36:** `Logger::error("Application initialization failed")`

**Coupling Type:** Static function calls (no injection)

**Refactoring Impact:**

- Static interface → Singleton instance with dependency injection
- Update all call sites to use `Logger::getInstance().info(...)`
- No domain model dependencies (clean separation)

---

### 2.2 GUI → Infrastructure Coupling

#### MainWindow.cpp Dependencies (CRITICAL INTEGRATION POINT)

**Export Functionality:**

```cpp
// MainWindow.cpp:459 - Current implementation
void MainWindow::exportResults() {
    // ...
    truss::core::ResultsExporter exporter;  // Direct instantiation
    truss::core::AnalysisResults results = m_analysisEngine->getLastResults();
    truss::core::ExportOptions options;
    // ... set options ...

    truss::core::ExportFormat format =
        truss::core::ResultsExporter::detectFormat(fileName.toStdString());

    if (exporter.exportResults(*getTruss(), results,
                               fileName.toStdString(), format, options)) {
        // Success
    }
}
```

**Refactoring Impact:**

- Replace direct instantiation with `ExporterFactory::create(format)`
- Update includes: `ResultsExporter.hpp` → `exporter_factory.hpp`
- Preserve format detection: Move to utility or keep static method
- **API Compatibility:** Must maintain `ExportFormat` enum and `ExportOptions` struct

**Post-Refactor Interface:**

```cpp
// MainWindow.cpp - Target implementation
void MainWindow::exportResults() {
    // ...
    auto format = ExporterFactory::detectFormat(fileName.toStdString());
    auto exporter = ExporterFactory::create(format);

    if (exporter->exportResults(results, fileName.toStdString())) {
        // Success
    }
}
```

---

#### ProjectFileManager Dependencies (Qt Framework Coupling)

**Location:** `src/gui/ProjectFileManager.{hpp,cpp}`

**Current Implementation:**

- Uses Qt JSON classes: `QJsonDocument`, `QJsonObject`, `QJsonArray`
- Serialization: `trussToJson()` converts domain model to QJsonObject
- Deserialization: `jsonToTruss()` reconstructs domain model
- File format: Custom `.truss` format (JSON structure)

**Phase 3 Decision:**

- **Option A (Conservative):** Leave as-is (Qt-specific GUI concern)
- **Option B (Aggressive):** Migrate to `IFileReader`/`IFileWriter` abstraction
- **Recommendation:** Option A - ProjectFileManager is GUI-layer specific, Phase 3 focuses on core infrastructure

**Rationale:**

- ProjectFileManager is tightly coupled to Qt (QFile, QJsonDocument)
- Moving to infrastructure requires Qt → std::filesystem migration (high risk)
- GUI layer can have its own I/O without violating architecture
- Phase 4 (Interface & Application) can revisit if needed

---

### 2.3 Dependency Inversion Violations

**Current Architecture:**

```
┌─────────────────┐
│   GUI Layer     │  (MainWindow, ProjectFileManager)
│                 │
│  ┌──────────┐   │
│  │ Results- │   │  Direct instantiation
│  │ Exporter │◄──┼──────────────────────
│  └──────────┘   │
└─────────────────┘

┌─────────────────┐
│  Core/Domain    │  (Truss, AnalysisEngine)
│                 │
│  ┌──────────┐   │
│  │ Logger   │◄──┼──────────────────────
│  └──────────┘   │  Static calls
└─────────────────┘
```

**Issues:**

- ✗ High-level modules (GUI) depend on low-level modules (ResultsExporter)
- ✗ No abstractions (interfaces) between layers
- ✗ Static dependencies prevent testing/mocking

**Target Architecture (Post-Phase 3):**

```
┌─────────────────────────────────┐
│         GUI Layer               │
│                                 │
│  Uses IResultsExporter ◄────────┼──────┐
│  Uses Logger::getInstance()     │      │
└─────────────────────────────────┘      │
                                         │ Dependency Injection
┌─────────────────────────────────┐      │
│     Infrastructure Layer        │      │
│                                 │      │
│  ┌──────────────────────┐       │      │
│  │ IResultsExporter     │◄──────┼──────┘
│  │ (interface)          │       │
│  └──────────────────────┘       │
│           ▲                     │
│           │                     │
│  ┌────────┴──────────┐          │
│  │ CSVExporter       │          │
│  │ JSONExporter      │          │
│  │ HTMLExporter      │          │
│  └───────────────────┘          │
│                                 │
│  ┌──────────────────────┐       │
│  │ Logger (singleton)   │       │
│  │ + ILogSink collection│       │
│  └──────────────────────┘       │
└─────────────────────────────────┘

┌─────────────────────────────────┐
│        Domain Layer             │
│  (Truss, AnalysisEngine, etc.)  │
│  No infrastructure dependencies │
└─────────────────────────────────┘
```

**Benefits:**

- ✓ GUI depends on abstractions (IResultsExporter, Logger interface)
- ✓ Domain layer completely decoupled from infrastructure
- ✓ Testable (can mock exporters and sinks)
- ✓ Extensible (add new exporters/sinks without modifying existing code)

---

## 3. API Preservation Requirements

### 3.1 Critical APIs to Preserve

#### A. ExportFormat Enum

**Location:** `src/core/ResultsExporter.hpp`  
**Current Definition:**

```cpp
enum class ExportFormat {
    CSV,
    TSV,
    JSON,
    XML,
    TXT,
    LaTeX,
    HTML
};
```

**Preservation Strategy:**

- ✓ Move to `src/infrastructure/export/export_types.hpp`
- ✓ Maintain all 7 format values (no deletions)
- ✓ Update GUI includes: `#include "export_types.hpp"`

---

#### B. ExportOptions Struct

**Location:** `src/core/ResultsExporter.hpp`  
**Current Definition:**

```cpp
struct ExportOptions {
    bool includeMetadata{true};
    bool includeGeometry{true};
    bool includeProperties{true};
    bool includeBoundaryConditions{true};
    bool includeLoads{true};
    bool includeDisplacements{true};
    bool includeMemberForces{true};
    bool includeReactions{true};
    bool includeStiffnessMatrix{false};
    bool useScientificNotation{false};
    bool prettyPrint{true};
    int precision{6};
    std::string delimiter{","};  // For CSV/TSV
};
```

**Preservation Strategy:**

- ✓ Move to `src/infrastructure/export/export_types.hpp`
- ✓ Maintain all 12 flags + precision/delimiter fields
- ✓ All concrete exporters accept `ExportOptions` as parameter
- ✓ Backward compatibility: Default values match current implementation

---

#### C. Logger Interface (Log Levels)

**Location:** `src/core/Logger.hpp`  
**Current Interface:**

```cpp
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical
};

class Logger {
public:
    static void trace(const std::string& message);
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);
};
```

**Preservation Strategy:**

- ✓ Maintain all 6 log levels (enum values)
- ✓ Maintain convenience methods (trace, debug, info, warn, error, critical)
- ✓ Change: Static methods → Instance methods via `Logger::getInstance()`
- ✓ Call site updates: `Logger::info(...)` → `Logger::getInstance().info(...)`

**Migration Path:**

1. Add `getInstance()` static method (singleton)
2. Keep static convenience wrappers that call instance methods
3. Update all 20 call sites to use `getInstance()`
4. Remove static wrappers in Phase 4 (breaking change, with deprecation warnings)

---

### 3.2 GUI Integration Points

#### Required Updates to MainWindow.cpp

**Line 7:** Include update

```cpp
// Current:
#include "ResultsExporter.hpp"

// Target:
#include "infrastructure/export/exporter_factory.hpp"
#include "infrastructure/export/export_types.hpp"
```

**Line 459:** Export method update

```cpp
// Current:
truss::core::ResultsExporter exporter;
truss::core::ExportFormat format =
    truss::core::ResultsExporter::detectFormat(fileName.toStdString());
if (exporter.exportResults(*getTruss(), results, fileName.toStdString(), format, options)) {
    // ...
}

// Target:
auto format = truss::infrastructure::ExporterFactory::detectFormat(fileName.toStdString());
auto exporter = truss::infrastructure::ExporterFactory::create(format);
if (exporter->exportResults(results, fileName.toStdString())) {
    // ...
}
```

**Critical Requirement:** Zero functional changes to user-facing export behavior

---

## 4. Refactoring Risks & Mitigation Strategies

### 4.1 High-Risk Areas

#### Risk 1: Export Functionality Regression (HIGH)

**Threat:** Decomposed exporters produce different output than monolithic implementation

**Mitigation:**

1. **Golden Master Testing:**
   - Generate reference output files with current `ResultsExporter`
   - Store in `tests/fixtures/export_golden/`
   - After refactor, byte-compare new output with golden files
2. **Test Strategy:**

   ```cpp
   // tests/unit/infrastructure/export/test_csv_exporter.cpp
   TEST(CSVExporter, OutputMatchesGoldenMaster) {
       // Load fixture truss
       auto truss = loadFixture("bridge_truss.json");
       auto results = runAnalysis(truss);

       // Export with new exporter
       CSVExporter exporter;
       exporter.exportResults(results, "output.csv");

       // Compare with golden master
       ASSERT_TRUE(filesMatch("output.csv",
                              "tests/fixtures/export_golden/bridge_truss.csv"));
   }
   ```

3. **Validation Checklist:**
   - [ ] CSV format: Delimiter, headers, row count match
   - [ ] JSON format: Structure, field names, precision match
   - [ ] XML format: Schema, element hierarchy match
   - [ ] HTML format: Table structure, styling match
   - [ ] LaTeX format: Document structure, commands match
   - [ ] Numerical precision: ±1e-6 tolerance for floating-point values

---

#### Risk 2: GUI Integration Breakage (HIGH)

**Threat:** MainWindow.cpp export functionality fails after refactor

**Mitigation:**

1. **Incremental Integration:**
   - Keep old `ResultsExporter` in codebase (deprecated) during Phase 3
   - Add new exporters alongside old implementation
   - Add feature flag: `USE_NEW_EXPORTERS` (CMake option)
   - Run both implementations in parallel during testing
2. **GUI Testing Protocol:**

   ```bash
   # Manual acceptance test
   1. Build with new exporters (USE_NEW_EXPORTERS=ON)
   2. Launch GUI application
   3. Load sample project (simple_truss.truss)
   4. Run analysis
   5. Export to all 6 formats (CSV, JSON, XML, HTML, LaTeX, TXT)
   6. Verify files open correctly in respective viewers
   7. Compare file sizes with legacy output (±5% tolerance)
   ```

3. **Rollback Plan:**
   - If critical issue found, revert MainWindow.cpp changes
   - Disable `USE_NEW_EXPORTERS` flag
   - Fix issues in isolation before re-integration

---

#### Risk 3: Logger Thread Safety Regression (MEDIUM)

**Threat:** Refactored Logger loses thread safety, causing race conditions

**Mitigation:**

1. **Concurrency Testing:**

   ```cpp
   // tests/unit/infrastructure/logging/test_logger_thread_safety.cpp
   TEST(Logger, ConcurrentWritesFromMultipleThreads) {
       Logger& logger = Logger::getInstance();

       const int numThreads = 10;
       const int messagesPerThread = 1000;

       std::vector<std::thread> threads;
       for (int i = 0; i < numThreads; ++i) {
           threads.emplace_back([&, i]() {
               for (int j = 0; j < messagesPerThread; ++j) {
                   logger.info("Thread " + std::to_string(i) +
                              " message " + std::to_string(j));
               }
           });
       }

       for (auto& t : threads) t.join();

       // Verify log file: should have exactly numThreads * messagesPerThread lines
       // No corrupted lines (mixed writes)
       ASSERT_EQ(countLogLines("TrussAnalysis2D.log"),
                 numThreads * messagesPerThread);
   }
   ```

2. **Implementation Requirements:**
   - Each `ILogSink` implementation must handle thread safety internally
   - `Logger` class uses `std::lock_guard` when iterating sink collection
   - FileSink uses per-sink mutex for file writes

---

#### Risk 4: Missing File I/O Test Coverage (MEDIUM)

**Threat:** New `IFileReader`/`IFileWriter` lack validation, causing data loss

**Mitigation:**

1. **Round-Trip Tests:**

   ```cpp
   TEST(JsonFileReaderWriter, RoundTripPreservesData) {
       // Create truss programmatically
       auto originalTruss = createTestTruss();

       // Write to file
       JsonFileWriter writer;
       writer.write(originalTruss, "temp_truss.json");

       // Read back
       JsonFileReader reader;
       auto loadedTruss = reader.read("temp_truss.json");

       // Compare
       ASSERT_TRUE(trussesEqual(originalTruss, loadedTruss));
   }
   ```

2. **Validation Criteria:**
   - All node positions preserved (±1e-9 tolerance)
   - All member connectivity preserved
   - Material properties preserved (Young's modulus, etc.)
   - Boundary conditions preserved (support types)
   - Applied loads preserved (force vectors)

---

#### Risk 5: Configuration Manager Integration Gaps (LOW)

**Threat:** ConfigManager not properly integrated, settings ignored

**Mitigation:**

1. **Integration Testing:**

   ```cpp
   TEST(ConfigManager, LoggerUsesConfiguredLevel) {
       // Write config file
       writeConfigFile("test_config.json", R"({
           "logging": {
               "level": "DEBUG",
               "outputFile": "/tmp/test.log"
           }
       })");

       // Load config
       ConfigManager::getInstance().loadFromFile("test_config.json");

       // Initialize logger
       Logger::getInstance().initialize();

       // Verify logger uses configured level
       Logger::getInstance().debug("Debug message");  // Should appear
       Logger::getInstance().trace("Trace message");   // Should NOT appear

       auto logContents = readFile("/tmp/test.log");
       ASSERT_TRUE(contains(logContents, "Debug message"));
       ASSERT_FALSE(contains(logContents, "Trace message"));
   }
   ```

2. **Environment Variable Override Testing:**
   - Test: `TRUSS_LOG_LEVEL=ERROR` overrides config file
   - Test: `TRUSS_OUTPUT_DIR=/custom/path` changes export location
   - Test: Missing env var falls back to config file
   - Test: Missing config falls back to hardcoded defaults

---

### 4.2 Risk Matrix

| Risk ID | Risk Description          | Probability | Impact | Severity   | Mitigation              |
| ------- | ------------------------- | ----------- | ------ | ---------- | ----------------------- |
| R1      | Export output regression  | Medium      | High   | **HIGH**   | Golden master tests     |
| R2      | GUI integration failure   | Low         | High   | **MEDIUM** | Incremental integration |
| R3      | Logger thread safety loss | Low         | Medium | **MEDIUM** | Concurrency tests       |
| R4      | File I/O data loss        | Low         | High   | **MEDIUM** | Round-trip tests        |
| R5      | ConfigManager ignored     | Low         | Low    | **LOW**    | Integration tests       |

---

## 5. Test Coverage Strategy

### 5.1 Current Test Coverage Gaps

**Infrastructure Test Status:** **0 tests** (as of v3.0.0)

**Grep Results:**

```bash
$ find tests -name "*.cpp" -exec grep -l "ResultsExporter\|Logger" {} \;
# No matches found
```

**Domain Test Status:** 72 passing tests (100% pass rate)

- Unit tests: 41 tests
- Integration tests: 31 tests

**Critical Gap:** Infrastructure components have zero automated test coverage despite being used in production

---

### 5.2 Phase 3 Test Requirements

#### A. ResultsExporter Tests (Minimum 30 tests)

**Test Categories:**

1. **Format Output Tests (6 tests - one per format)**
   - `test_csv_exporter.cpp`
   - `test_json_exporter.cpp`
   - `test_xml_exporter.cpp`
   - `test_html_exporter.cpp`
   - `test_latex_exporter.cpp`
   - `test_text_exporter.cpp`

2. **Golden Master Tests (6 tests)**
   - Compare new exporter output with reference files
   - Byte-level comparison for text formats
   - Numerical tolerance (±1e-6) for floating-point values

3. **Edge Case Tests (12 tests)**
   - Empty truss (no nodes/members)
   - Single-node truss (no members)
   - Large truss (1000+ nodes)
   - Invalid file path (permission denied)
   - Disk full scenario (write failure)
   - Unicode characters in truss name
   - Special characters in file path
   - Zero-length members
   - Duplicate node IDs
   - NaN/Inf values in results
   - Very small values (underflow)
   - Very large values (overflow)

4. **Options Testing (6 tests)**
   - All flags enabled
   - All flags disabled
   - Custom precision (0, 3, 15)
   - Custom delimiter (tab, semicolon)
   - Scientific notation vs fixed-point
   - Pretty-print vs compact

**Test File Structure:**

```
tests/unit/infrastructure/export/
├── test_csv_exporter.cpp          (5 tests)
├── test_json_exporter.cpp         (5 tests)
├── test_xml_exporter.cpp          (5 tests)
├── test_html_exporter.cpp         (5 tests)
├── test_latex_exporter.cpp        (5 tests)
├── test_text_exporter.cpp         (5 tests)
└── test_exporter_factory.cpp      (5 tests)
```

---

#### B. Logger Tests (Minimum 15 tests)

**Test Categories:**

1. **Sink Tests (9 tests)**
   - `test_console_sink.cpp` (3 tests: info, error, custom format)
   - `test_file_sink.cpp` (4 tests: write, append, rotation, permissions)
   - `test_syslog_sink.cpp` (2 tests: connection, format)

2. **Logger Core Tests (6 tests)**
   - `test_logger.cpp` (6 tests: level filtering, multiple sinks, thread safety, add/remove sinks, initialization, shutdown)

**Test File Structure:**

```
tests/unit/infrastructure/logging/
├── test_logger.cpp               (6 tests)
├── test_console_sink.cpp         (3 tests)
├── test_file_sink.cpp            (4 tests)
└── test_syslog_sink.cpp          (2 tests)
```

---

#### C. File I/O Tests (Minimum 10 tests)

**Test Categories:**

1. **Reader Tests (5 tests)**
   - Valid JSON file
   - Malformed JSON (syntax error)
   - Missing required fields
   - Invalid node IDs
   - Invalid member connectivity

2. **Writer Tests (5 tests)**
   - Write valid truss
   - Write to read-only location (failure)
   - Round-trip preservation
   - Large file (100k nodes)
   - Unicode file path

**Test File Structure:**

```
tests/unit/infrastructure/io/
├── test_json_file_reader.cpp     (5 tests)
└── test_json_file_writer.cpp     (5 tests)
```

---

#### D. Configuration Manager Tests (Minimum 8 tests)

**Test Categories:**

1. **Config Loading (4 tests)**
   - Load valid config file
   - Load invalid JSON (syntax error)
   - Missing config file (use defaults)
   - Environment variable override

2. **Config Access (4 tests)**
   - Get string value
   - Get integer value
   - Get boolean value
   - Get missing key (default value)

**Test File Structure:**

```
tests/unit/infrastructure/config/
└── test_config_manager.cpp       (8 tests)
```

---

### 5.3 Test Coverage Target

**Phase 3 Test Goals:**

- **Minimum:** 63 new infrastructure tests (exporters: 30, logger: 15, I/O: 10, config: 8)
- **Target Coverage:** 80% line coverage for infrastructure layer
- **Critical Paths:** 100% coverage for error handling (file write failures, invalid input)

**Validation Command:**

```bash
# Run all Phase 3 tests
ctest --test-dir build -R "infrastructure_.*"

# Expected output:
# 63/63 tests passed (100%)
```

---

## 6. Implementation Task Breakdown

### Task 3.1: Refactor ResultsExporter (Strategy Pattern)

**Effort:** 10 hours  
**Priority:** Critical  
**Dependencies:** None

**Subtasks:**

1. ✅ **Create directory structure** (15 min)

   ```bash
   mkdir -p src/infrastructure/export
   mkdir -p tests/unit/infrastructure/export
   mkdir -p tests/fixtures/export_golden
   ```

2. ✅ **Define interfaces** (1 hour)
   - `export_types.hpp` - Move ExportFormat, ExportOptions
   - `exporter.hpp` - IResultsExporter interface
   - `exporter_factory.hpp` - Factory pattern implementation

3. ✅ **Generate golden master files** (1 hour)

   ```bash
   # Run current ResultsExporter on test fixtures
   ./test_export --generate-golden
   # Outputs: tests/fixtures/export_golden/*.{csv,json,xml,html,tex,txt}
   ```

4. ✅ **Implement CSV exporter** (1.5 hours)
   - Copy logic from `ResultsExporter::exportToCSV()`
   - Create `csv_exporter.{hpp,cpp}`
   - Write 5 unit tests
   - Run golden master comparison

5. ✅ **Implement JSON exporter** (1.5 hours)
   - Copy logic from `ResultsExporter::exportToJSON()`
   - Create `json_exporter.{hpp,cpp}`
   - Write 5 unit tests
   - Run golden master comparison

6. ✅ **Implement XML exporter** (1 hour)
   - Copy logic from `ResultsExporter::exportToXML()`
   - Create `xml_exporter.{hpp,cpp}`
   - Write 5 unit tests
   - Run golden master comparison

7. ✅ **Implement HTML exporter** (1 hour)
   - Copy logic from `ResultsExporter::exportToHTML()`
   - Create `html_exporter.{hpp,cpp}`
   - Write 5 unit tests
   - Run golden master comparison

8. ✅ **Implement LaTeX exporter** (1 hour)
   - Copy logic from `ResultsExporter::exportToLaTeX()`
   - Create `latex_exporter.{hpp,cpp}`
   - Write 5 unit tests
   - Run golden master comparison

9. ✅ **Implement factory** (1 hour)
   - `exporter_factory.cpp` - Create factory methods
   - Move `detectFormat()` logic to factory
   - Write 5 factory tests

10. ✅ **Update MainWindow.cpp** (30 min)
    - Update includes
    - Replace direct instantiation with factory
    - Test GUI export functionality manually

11. ✅ **Delete old implementation** (15 min)
    ```bash
    git rm src/core/ResultsExporter.{hpp,cpp}
    ```

**Validation Criteria:**

- [ ] All 30 exporter tests pass
- [ ] Golden master tests pass (byte-identical output)
- [ ] GUI export works for all 6 formats
- [ ] Old `ResultsExporter.*` deleted
- [ ] CMakeLists.txt updated

---

### Task 3.2: Implement File I/O Services

**Effort:** 8 hours  
**Priority:** High  
**Dependencies:** None

**Subtasks:**

1. ✅ **Create directory structure** (10 min)

   ```bash
   mkdir -p src/infrastructure/io
   mkdir -p tests/unit/infrastructure/io
   mkdir -p tests/fixtures/truss_files
   ```

2. ✅ **Define interfaces** (1 hour)
   - `file_reader.hpp` - IFileReader interface
   - `file_writer.hpp` - IFileWriter interface

3. ✅ **Create test fixtures** (1 hour)
   - `simple_truss.json` - 4 nodes, 5 members
   - `bridge_truss.json` - Complex structure
   - `invalid_truss.json` - Missing fields
   - `malformed.json` - Syntax errors

4. ✅ **Implement JsonFileReader** (2 hours)
   - `json_file_reader.{hpp,cpp}`
   - JSON parsing (use existing logic from `ProjectFileManager`)
   - Error handling (invalid JSON, missing fields)
   - Write 5 unit tests

5. ✅ **Implement JsonFileWriter** (2 hours)
   - `json_file_writer.{hpp,cpp}`
   - JSON serialization
   - Error handling (permission denied, disk full)
   - Write 5 unit tests

6. ✅ **Round-trip testing** (1 hour)
   - Write→Read→Write cycle
   - Compare original and round-tripped files
   - Numerical tolerance tests

7. ✅ **Documentation** (1 hour)
   - API documentation (Doxygen comments)
   - Usage examples

**Validation Criteria:**

- [ ] All 10 I/O tests pass
- [ ] Round-trip tests preserve data (±1e-9 tolerance)
- [ ] Invalid files handled gracefully (no crashes)
- [ ] API documentation complete

---

### Task 3.3: Implement Logging System

**Effort:** 6 hours  
**Priority:** Medium  
**Dependencies:** None

**Subtasks:**

1. ✅ **Create directory structure** (10 min)

   ```bash
   mkdir -p src/infrastructure/logging
   mkdir -p tests/unit/infrastructure/logging
   ```

2. ✅ **Define interface** (30 min)
   - `log_sink.hpp` - ILogSink interface

3. ✅ **Implement ConsoleSink** (1 hour)
   - `console_sink.{hpp,cpp}`
   - stdout/stderr routing based on log level
   - Write 3 unit tests

4. ✅ **Implement FileSink** (1.5 hours)
   - `file_sink.{hpp,cpp}`
   - File rotation (optional)
   - Buffering for performance
   - Write 4 unit tests

5. ✅ **Implement SyslogSink** (1 hour)
   - `syslog_sink.{hpp,cpp}` (Linux only)
   - syslog API integration
   - Write 2 unit tests

6. ✅ **Refactor Logger class** (1.5 hours)
   - Move `src/core/Logger.*` → `src/infrastructure/logging/`
   - Replace static file handle with sink collection
   - Add `addSink()`, `removeSink()` methods
   - Maintain backward compatibility (static wrappers)
   - Write 6 logger tests

7. ✅ **Update call sites** (30 min)
   - `main.cpp` - Add console + file sinks during initialization
   - `Application.cpp` - No changes (static wrappers still work)

**Validation Criteria:**

- [ ] All 15 logging tests pass
- [ ] Thread safety validated (concurrent write test)
- [ ] Multiple sinks write to different destinations
- [ ] Log level filtering works correctly
- [ ] Backward compatibility maintained (old static API still works)

---

### Task 3.4: Implement Configuration Manager

**Effort:** 6 hours  
**Priority:** Low  
**Dependencies:** None

**Subtasks:**

1. ✅ **Create directory structure** (10 min)

   ```bash
   mkdir -p src/infrastructure/config
   mkdir -p config
   mkdir -p tests/unit/infrastructure/config
   ```

2. ✅ **Define interface** (1 hour)
   - `config_manager.{hpp,cpp}` - Singleton pattern
   - Template `get<T>()` method for type-safe access

3. ✅ **Implement JSON parser** (2 hours)
   - `config_parser.{hpp,cpp}` - Parse JSON config
   - Handle nested objects
   - Type conversion (string→int, string→bool, etc.)

4. ✅ **Create default config** (1 hour)

   ```json
   // config/default.json
   {
     "logging": {
       "level": "INFO",
       "outputFile": "TrussAnalysis2D.log",
       "enableConsole": true,
       "enableFile": true
     },
     "analysis": {
       "defaultSolver": "DirectSolver",
       "convergenceTolerance": 1e-9,
       "maxIterations": 1000
     },
     "export": {
       "defaultPrecision": 6,
       "defaultFormat": "CSV"
     }
   }
   ```

5. ✅ **Implement environment overrides** (1 hour)
   - `TRUSS_LOG_LEVEL` overrides `logging.level`
   - `TRUSS_OUTPUT_DIR` overrides export directory
   - Write 2 tests for env var priority

6. ✅ **Write tests** (1 hour)
   - 4 config loading tests
   - 4 config access tests

**Validation Criteria:**

- [ ] All 8 config tests pass
- [ ] Default config loads successfully
- [ ] Environment variables override config file
- [ ] Missing config falls back to defaults
- [ ] Invalid JSON handled gracefully

---

## 7. Validation & Acceptance Criteria

### 7.1 Phase 3 Completion Checklist

#### Directory Structure

- [ ] `src/infrastructure/` directory created
- [ ] `src/infrastructure/export/` with 6 concrete exporters + factory
- [ ] `src/infrastructure/io/` with IFileReader/IFileWriter + JSON implementations
- [ ] `src/infrastructure/logging/` with Logger + 3 sinks
- [ ] `src/infrastructure/config/` with ConfigManager
- [ ] Old `src/core/Logger.*` deleted
- [ ] Old `src/core/ResultsExporter.*` deleted

#### Test Coverage

- [ ] 30+ exporter tests passing
- [ ] 15+ logger tests passing
- [ ] 10+ I/O tests passing
- [ ] 8+ config tests passing
- [ ] **Total:** 63+ new infrastructure tests
- [ ] All 72 existing domain tests still passing (no regression)
- [ ] **Grand Total:** 135+ tests passing (100% pass rate)

#### Functional Validation

- [ ] GUI export functionality works for all 6 formats
- [ ] Golden master tests pass (byte-identical output)
- [ ] Logger writes to multiple sinks correctly
- [ ] Log level filtering works
- [ ] Thread safety validated (no race conditions)
- [ ] ConfigManager loads default config
- [ ] Environment variables override config

#### Code Quality

- [ ] All infrastructure code documented (Doxygen comments)
- [ ] No compiler warnings (`-Wall -Wextra -Werror`)
- [ ] No memory leaks (valgrind clean)
- [ ] ClangTidy checks pass
- [ ] Code review completed (2 reviewers)

#### Build System

- [ ] CMakeLists.txt updated with new infrastructure targets
- [ ] `src/infrastructure/CMakeLists.txt` created
- [ ] Library dependencies declared correctly
- [ ] Install targets updated

#### Documentation

- [ ] Phase 3 tasks marked complete in [REFACTORING_PROGRESS.md](../REFACTORING_PROGRESS.md)
- [ ] API documentation generated (Doxygen)
- [ ] Architecture diagrams updated (UML class diagrams)
- [ ] Migration guide written (for future developers)

---

### 7.2 Acceptance Test Protocol

#### Manual GUI Testing (30 minutes)

1. **Build Phase 3 Branch:**

   ```bash
   git checkout phase-3-infrastructure
   mkdir build && cd build
   cmake .. -DBUILD_GUI=ON
   make -j8
   ```

2. **Launch GUI:**

   ```bash
   ./TrussAnalysis2D_GUI
   ```

3. **Test Export Workflow:**
   - Load sample project: `File → Open → tests/fixtures/truss_files/bridge_truss.truss`
   - Run analysis: `Analyze → Run Analysis`
   - Export CSV: `File → Export Results → bridge_results.csv`
   - Export JSON: `File → Export Results → bridge_results.json`
   - Export XML: `File → Export Results → bridge_results.xml`
   - Export HTML: `File → Export Results → bridge_results.html`
   - Export LaTeX: `File → Export Results → bridge_results.tex`
   - Export TXT: `File → Export Results → bridge_results.txt`

4. **Verify Output Files:**

   ```bash
   # Check file sizes (should be non-zero)
   ls -lh bridge_results.*

   # Open CSV in spreadsheet (LibreOffice Calc / Excel)
   libreoffice --calc bridge_results.csv

   # Open HTML in browser
   firefox bridge_results.html

   # Compile LaTeX
   pdflatex bridge_results.tex
   ```

5. **Log Verification:**

   ```bash
   # Check log file exists
   cat TrussAnalysis2D.log | head -20

   # Verify console output (should show INFO/WARN/ERROR messages)
   ```

**Acceptance Criteria:**

- ✓ All 6 formats export successfully (no errors)
- ✓ File sizes match expected ranges (±10% of legacy output)
- ✓ CSV opens in spreadsheet software
- ✓ HTML renders correctly in browser
- ✓ LaTeX compiles without errors
- ✓ Log file contains analysis messages

---

#### Automated Test Suite (10 minutes)

```bash
# Run all tests
cd build
ctest --output-on-failure

# Expected output:
# 100% tests passed, 0 tests failed out of 135
#
# Label Time Summary:
# domain              =   5.23 sec*proc (72 tests)
# infrastructure      =   2.87 sec*proc (63 tests)
#
# Total Test time (real) =   8.10 sec
```

**Acceptance Criteria:**

- ✓ 135/135 tests pass (100% pass rate)
- ✓ No test failures
- ✓ No memory leaks reported
- ✓ Test execution time < 15 seconds

---

#### Code Quality Validation (15 minutes)

```bash
# Static analysis
clang-tidy src/infrastructure/**/*.cpp -- -Isrc -std=c++20

# Memory leak check
valgrind --leak-check=full --show-leak-kinds=all ./build/tests/unit_tests

# Code coverage
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
firefox coverage_html/index.html
```

**Acceptance Criteria:**

- ✓ Zero clang-tidy warnings
- ✓ Zero memory leaks (valgrind: "All heap blocks were freed")
- ✓ Infrastructure layer: ≥80% line coverage

---

## 8. Post-Phase 3 Architecture

### 8.1 Final Directory Structure

```
src/
├── core/
│   ├── Application.{hpp,cpp}          # Uses Logger, ConfigManager
│   ├── analysis/                      # Domain layer (no changes)
│   │   ├── AnalysisEngine.{hpp,cpp}
│   │   ├── StiffnessAssembler.{hpp,cpp}
│   │   ├── BoundaryConditionHandler.{hpp,cpp}
│   │   └── solvers/
│   │       ├── ILinearSolver.hpp
│   │       ├── DirectSolver.{hpp,cpp}
│   │       ├── IterativeSolver.{hpp,cpp}
│   │       └── SolverFactory.{hpp,cpp}
│   └── model/                         # Domain layer (no changes)
│       ├── Truss.{hpp,cpp}
│       ├── Node.{hpp,cpp}
│       ├── Member.{hpp,cpp}
│       └── Types.hpp
│
├── infrastructure/                    # ✨ NEW
│   ├── export/                        # ✨ NEW
│   │   ├── export_types.hpp           # ExportFormat, ExportOptions
│   │   ├── exporter.hpp               # IResultsExporter interface
│   │   ├── exporter_factory.{hpp,cpp}
│   │   ├── csv_exporter.{hpp,cpp}
│   │   ├── json_exporter.{hpp,cpp}
│   │   ├── xml_exporter.{hpp,cpp}
│   │   ├── html_exporter.{hpp,cpp}
│   │   ├── latex_exporter.{hpp,cpp}
│   │   └── CMakeLists.txt
│   │
│   ├── io/                            # ✨ NEW
│   │   ├── file_reader.hpp            # IFileReader interface
│   │   ├── file_writer.hpp            # IFileWriter interface
│   │   ├── json_file_reader.{hpp,cpp}
│   │   ├── json_file_writer.{hpp,cpp}
│   │   └── CMakeLists.txt
│   │
│   ├── logging/                       # ✨ NEW (moved from core)
│   │   ├── logger.{hpp,cpp}           # Refactored with sinks
│   │   ├── log_sink.hpp               # ILogSink interface
│   │   ├── console_sink.{hpp,cpp}
│   │   ├── file_sink.{hpp,cpp}
│   │   ├── syslog_sink.{hpp,cpp}
│   │   └── CMakeLists.txt
│   │
│   ├── config/                        # ✨ NEW
│   │   ├── config_manager.{hpp,cpp}
│   │   ├── config_parser.{hpp,cpp}
│   │   └── CMakeLists.txt
│   │
│   └── CMakeLists.txt                 # Infrastructure layer CMake
│
└── gui/
    ├── MainWindow.{hpp,cpp}           # Updated: uses ExporterFactory
    ├── InteractiveDrawingWidget.{hpp,cpp}
    ├── DeformedTrussWidget.{hpp,cpp}
    ├── ProjectFileManager.{hpp,cpp}   # No changes (Phase 3)
    └── ...
```

---

### 8.2 Dependency Graph (Post-Phase 3)

```
┌─────────────────────────────────────────────────────────────┐
│                       GUI Layer                             │
│                                                             │
│  MainWindow ──uses──► ExporterFactory                       │
│                       (infrastructure::export)              │
│                                                             │
│  MainWindow ──uses──► Logger::getInstance()                 │
│                       (infrastructure::logging)             │
│                                                             │
│  ProjectFileManager  (Qt-specific, unchanged)               │
└─────────────────────────────────────────────────────────────┘
                        │ │
                        │ └──uses──► IResultsExporter (interface)
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│                  Infrastructure Layer                       │
│                                                             │
│  ┌───────────────────────────────────────────┐              │
│  │ export/                                   │              │
│  │   IResultsExporter ◄──implements──┬───────┤              │
│  │                                   │       │              │
│  │   CSVExporter                     │       │              │
│  │   JSONExporter                    │       │              │
│  │   XMLExporter                     │       │              │
│  │   HTMLExporter                    │       │              │
│  │   LaTeXExporter                   │       │              │
│  │                                   │       │              │
│  │   ExporterFactory ────creates─────┘       │              │
│  └───────────────────────────────────────────┘              │
│                                                             │
│  ┌───────────────────────────────────────────┐              │
│  │ io/                                       │              │
│  │   IFileReader / IFileWriter               │              │
│  │   JsonFileReader / JsonFileWriter         │              │
│  └───────────────────────────────────────────┘              │
│                                                             │
│  ┌───────────────────────────────────────────┐              │
│  │ logging/                                  │              │
│  │   Logger (singleton)                      │              │
│  │   ├── ILogSink ◄──implements──┬───────────┤              │
│  │   │                           │           │              │
│  │   ConsoleSink                 │           │              │
│  │   FileSink                    │           │              │
│  │   SyslogSink                  │           │              │
│  └───────────────────────────────────────────┘              │
│                                                             │
│  ┌───────────────────────────────────────────┐              │
│  │ config/                                   │              │
│  │   ConfigManager (singleton)               │              │
│  │   ConfigParser                            │              │
│  └───────────────────────────────────────────┘              │
└─────────────────────────────────────────────────────────────┘
                        │
                        │ (no dependencies)
                        ▼
┌─────────────────────────────────────────────────────────────┐
│                     Domain Layer                            │
│                                                             │
│  ┌─────────────────┐          ┌────────────────────┐        │
│  │ model/          │          │ analysis/          │        │
│  │   Truss         │◄──uses───┤ AnalysisEngine     │        │
│  │   Node          │          │ StiffnessAssembler │        │
│  │   Member        │          │ Solvers (Strategy) │        │
│  └─────────────────┘          └────────────────────┘        │
│                                                             │
│  (No infrastructure dependencies - clean separation)        │
└─────────────────────────────────────────────────────────────┘
```

**Key Principles:**

- ✓ Infrastructure layer has **no dependencies on domain**
- ✓ GUI layer depends on infrastructure **interfaces** (not concrete classes)
- ✓ Domain layer remains **completely decoupled** from infrastructure
- ✓ Dependency Inversion Principle enforced (depend on abstractions)

---

## 9. Open Questions & Decisions

### 9.1 ProjectFileManager Migration Decision

**Question:** Should `ProjectFileManager` (GUI layer) be migrated to use `IFileReader`/`IFileWriter`?

**Current State:**

- `ProjectFileManager` is Qt-specific (uses QJsonDocument, QFile, QJsonObject)
- Provides `.truss` file format (custom JSON structure)
- Tightly coupled to GUI layer
- Only used by MainWindow for project save/load

**Options:**

**Option A (Conservative - RECOMMENDED):**

- Leave `ProjectFileManager` unchanged in Phase 3
- Reason: Qt-specific GUI concern, not core infrastructure
- Defer to Phase 4 (Interface & Application) if needed

**Option B (Aggressive):**

- Migrate `ProjectFileManager` to use `JsonFileReader`/`JsonFileWriter`
- Requires Qt → std::filesystem migration
- Higher risk, more work (additional 4-6 hours)

**Decision:** **Option A** - Preserve ProjectFileManager in Phase 3

- **Rationale:** Phase 3 focuses on core infrastructure separation; GUI-layer I/O can remain Qt-specific
- **Future:** Phase 4 can revisit if unified I/O abstraction is desired

---

### 9.2 Logger Backward Compatibility Strategy

**Question:** Should Phase 3 maintain static Logger methods (`Logger::info(...)`) or force migration to instance methods (`Logger::getInstance().info(...)`)?

**Options:**

**Option A (Backward Compatible - RECOMMENDED):**

- Keep static convenience wrappers that delegate to instance
- Example:
  ```cpp
  class Logger {
  public:
      static Logger& getInstance();

      // Instance methods (preferred)
      void info(const std::string& message);
      void warn(const std::string& message);

      // Static wrappers (deprecated, for backward compatibility)
      static void info_static(const std::string& msg) {
          getInstance().info(msg);
      }
  };
  ```
- Update call sites incrementally (Phase 3: optional, Phase 4: mandatory)
- Add deprecation warnings in Phase 4

**Option B (Breaking Change):**

- Remove all static methods immediately
- Force all call sites to use `Logger::getInstance().info(...)`
- Higher risk of breaking existing code

**Decision:** **Option A** - Maintain backward compatibility

- **Rationale:** Minimize Phase 3 scope, reduce risk, allow gradual migration
- **Migration Path:** Update call sites opportunistically, deprecate in Phase 4

---

### 9.3 Export Format Extensibility

**Question:** Should the export system support plugin-style format extensions?

**Current Plan:** Fixed set of 5 exporters (CSV, JSON, XML, HTML, LaTeX)

**Options:**

**Option A (Fixed Set - RECOMMENDED):**

- All exporters compiled into library
- Factory has hardcoded format→exporter mapping
- Simple, predictable, sufficient for current needs

**Option B (Plugin System):**

- Exporters loaded dynamically (shared libraries)
- Factory uses plugin registry
- Higher complexity, more flexibility

**Decision:** **Option A** - Fixed set of exporters

- **Rationale:** No requirement for user-defined formats, YAGNI principle
- **Future:** Can add plugin system in Phase 5 (Build & Deployment) if needed

---

## 10. Next Steps & Implementation Roadmap

### 10.1 Phase 3 Execution Plan

**Week 1: Exporters & I/O (18 hours)**

- Day 1-2: Task 3.1 - Refactor ResultsExporter (10 hours)
- Day 3-4: Task 3.2 - Implement File I/O Services (8 hours)

**Week 2: Logging & Config (12 hours)**

- Day 1: Task 3.3 - Implement Logging System (6 hours)
- Day 2: Task 3.4 - Implement Configuration Manager (6 hours)

**Total Duration:** 2 weeks (30 hours)

---

### 10.2 Immediate Next Actions (Post-Orientation)

1. **Create Feature Branch:**

   ```bash
   git checkout -b phase-3-infrastructure
   ```

2. **Create Directory Structure:**

   ```bash
   mkdir -p src/infrastructure/{export,io,logging,config}
   mkdir -p tests/unit/infrastructure/{export,io,logging,config}
   mkdir -p tests/fixtures/{export_golden,truss_files}
   mkdir -p config
   ```

3. **Generate Golden Master Files:**

   ```bash
   # Run existing ResultsExporter on test fixtures
   ./build/test_export --output-dir tests/fixtures/export_golden/
   ```

4. **Begin Task 3.1 (ResultsExporter):**
   - Create `src/infrastructure/export/export_types.hpp`
   - Create `src/infrastructure/export/exporter.hpp`
   - Create `src/infrastructure/export/csv_exporter.{hpp,cpp}`
   - Write first 5 unit tests

---

### 10.3 Success Metrics

**Quantitative:**

- [ ] 63+ new infrastructure tests added
- [ ] 135+ total tests passing (domain + infrastructure)
- [ ] 0 test failures
- [ ] 0 memory leaks (valgrind clean)
- [ ] ≥80% infrastructure line coverage
- [ ] ≤15 sec test execution time

**Qualitative:**

- [ ] Clean separation of concerns (infrastructure isolated)
- [ ] Domain layer completely decoupled from infrastructure
- [ ] GUI integration successful (export functionality works)
- [ ] Code review approval (2 reviewers)
- [ ] No user-facing behavior changes (backward compatible)

**Documentation:**

- [ ] API documentation complete (Doxygen)
- [ ] Architecture diagrams updated
- [ ] Phase 3 orientation summary approved
- [ ] Migration guide written

---

## 11. Conclusion

This orientation summary provides comprehensive understanding of:

1. **Current State:** Infrastructure components co-located with domain, monolithic design, zero test coverage
2. **Target State:** Separated infrastructure layer, Strategy/Sink patterns, 63+ new tests
3. **Coupling Analysis:** GUI→Infrastructure and Infrastructure→Domain dependencies mapped
4. **API Preservation:** ExportFormat, ExportOptions, Logger interface backward compatible
5. **Risk Mitigation:** Golden master tests, incremental integration, thread safety validation
6. **Implementation Plan:** 30 hours over 2 weeks, 4 major tasks

**Phase 3 Ready to Proceed:** All prerequisites satisfied, orientation complete, risks identified, mitigation strategies in place.

**Next Action:** Await approval to begin Task 3.1 (ResultsExporter refactoring) with golden master test generation.

---

**Document Status:** ✅ COMPLETE - Ready for Implementation  
**Last Updated:** 2026-02-07  
**Approval Required:** Project Lead / Technical Architect
