# Build System Design: 2D Truss Analysis

**Date:** February 4, 2026  
**Build Tools:** CMake 3.20+, Make, Ninja  
**Target Platform:** Linux (Ubuntu 22.04+)

---

## 1. Build System Philosophy

**Objectives:**

1. **Simplicity** - Easy to build for developers
2. **Reproducibility** - Consistent builds across environments
3. **Performance** - Fast incremental builds
4. **Modularity** - Clear separation of concerns
5. **Professional** - Industry-standard practices

---

## 2. Root Makefile

```makefile
# Makefile - Professional build orchestration
.DEFAULT_GOAL := help
.PHONY: help all build test clean install docker-build docker-run lint format docs

# Build configuration
BUILD_DIR := build
BUILD_TYPE := Release
CMAKE_GENERATOR := Ninja
INSTALL_PREFIX := /usr/local

# Tool paths
CMAKE := cmake
CTEST := ctest
CLANG_FORMAT := clang-format
CLANG_TIDY := clang-tidy
CPPCHECK := cppcheck

# Color output
GREEN := \033[0;32m
YELLOW := \033[0;33m
RESET := \033[0m

##@ General

help: ## Display this help message
	@echo "$(GREEN)2D Truss Analysis - Build System$(RESET)"
	@echo ""
	@awk 'BEGIN {FS = ":.*##"; printf "Usage: make $(YELLOW)<target>$(RESET)\n"} \
		/^[a-zA-Z_-]+:.*?##/ { printf "  $(YELLOW)%-20s$(RESET) %s\n", $$1, $$2 } \
		/^##@/ { printf "\n$(GREEN)%s$(RESET)\n", substr($$0, 5) } ' $(MAKEFILE_LIST)

##@ Build

all: build test ## Build and test everything

configure: ## Configure CMake build system
	@echo "$(GREEN)Configuring build system...$(RESET)"
	$(CMAKE) -B $(BUILD_DIR) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DBUILD_TESTING=ON \
		-DENABLE_WARNINGS=ON

build: configure ## Build the project
	@echo "$(GREEN)Building project...$(RESET)"
	$(CMAKE) --build $(BUILD_DIR) -j$$(nproc)

build-debug: ## Build with debug symbols
	@echo "$(GREEN)Building with debug symbols...$(RESET)"
	$(CMAKE) -B $(BUILD_DIR) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_TESTING=ON
	$(CMAKE) --build $(BUILD_DIR) -j$$(nproc)

build-release: ## Build optimized release
	@echo "$(GREEN)Building optimized release...$(RESET)"
	$(CMAKE) -B $(BUILD_DIR) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_TESTING=OFF
	$(CMAKE) --build $(BUILD_DIR) -j$$(nproc)

rebuild: clean build ## Clean and rebuild

##@ Testing

test: build ## Run all tests
	@echo "$(GREEN)Running tests...$(RESET)"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

test-verbose: build ## Run tests with verbose output
	@echo "$(GREEN)Running tests (verbose)...$(RESET)"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure --verbose

test-unit: build ## Run only unit tests
	@echo "$(GREEN)Running unit tests...$(RESET)"
	cd $(BUILD_DIR) && $(CTEST) -R "unit_.*" --output-on-failure

test-integration: build ## Run only integration tests
	@echo "$(GREEN)Running integration tests...$(RESET)"
	cd $(BUILD_DIR) && $(CTEST) -R "integration_.*" --output-on-failure

test-coverage: ## Generate test coverage report
	@echo "$(GREEN)Generating coverage report...$(RESET)"
	$(CMAKE) -B $(BUILD_DIR) -G $(CMAKE_GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DENABLE_COVERAGE=ON
	$(CMAKE) --build $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CTEST)
	lcov --capture --directory $(BUILD_DIR) --output-file $(BUILD_DIR)/coverage.info
	lcov --remove $(BUILD_DIR)/coverage.info '/usr/*' '*/tests/*' --output-file $(BUILD_DIR)/coverage.info
	genhtml $(BUILD_DIR)/coverage.info --output-directory $(BUILD_DIR)/coverage_html
	@echo "$(GREEN)Coverage report: $(BUILD_DIR)/coverage_html/index.html$(RESET)"

##@ Quality Assurance

lint: ## Run static analysis with clang-tidy
	@echo "$(GREEN)Running clang-tidy...$(RESET)"
	find src include -name "*.cpp" -o -name "*.hpp" | \
		xargs $(CLANG_TIDY) -p $(BUILD_DIR)

format: ## Format code with clang-format
	@echo "$(GREEN)Formatting code...$(RESET)"
	find src include tests -name "*.cpp" -o -name "*.hpp" | \
		xargs $(CLANG_FORMAT) -i -style=file

format-check: ## Check code formatting
	@echo "$(GREEN)Checking code format...$(RESET)"
	find src include tests -name "*.cpp" -o -name "*.hpp" | \
		xargs $(CLANG_FORMAT) --dry-run -Werror -style=file

cppcheck: ## Run cppcheck static analysis
	@echo "$(GREEN)Running cppcheck...$(RESET)"
	$(CPPCHECK) --enable=all --suppress=missingIncludeSystem \
		--error-exitcode=1 \
		-I include src/

##@ Installation

install: build ## Install the application
	@echo "$(GREEN)Installing to $(INSTALL_PREFIX)...$(RESET)"
	$(CMAKE) --install $(BUILD_DIR)

uninstall: ## Uninstall the application
	@echo "$(GREEN)Uninstalling...$(RESET)"
	xargs rm -fv < $(BUILD_DIR)/install_manifest.txt

install-local: ## Install to ~/.local
	@echo "$(GREEN)Installing to ~/.local...$(RESET)"
	$(CMAKE) -B $(BUILD_DIR) -DCMAKE_INSTALL_PREFIX=$$HOME/.local
	$(CMAKE) --build $(BUILD_DIR)
	$(CMAKE) --install $(BUILD_DIR)

##@ Docker

docker-build: ## Build Docker container
	docker build -t truss-analysis:latest -f docker/Dockerfile .

docker-run: ## Run in Docker container
	docker run --rm -it \
		-v $$(pwd)/examples:/data/input:ro \
		-v $$(pwd)/output:/data/output:rw \
		truss-analysis:latest

docker-dev: ## Start development container
	docker-compose run --rm truss-dev

docker-test: ## Test Docker container
	docker build -t truss-analysis:test -f docker/Dockerfile .
	docker run --rm truss-analysis:test --version

##@ Cleanup

clean: ## Remove build artifacts
	@echo "$(GREEN)Cleaning build directory...$(RESET)"
	rm -rf $(BUILD_DIR)

clean-all: clean ## Remove all generated files
	@echo "$(GREEN)Cleaning all generated files...$(RESET)"
	find . -name "*.o" -o -name "*.a" -o -name "*.so" | xargs rm -f
	rm -rf docs/html docs/latex

##@ Documentation

docs: ## Generate documentation with Doxygen
	@echo "$(GREEN)Generating documentation...$(RESET)"
	doxygen Doxyfile
	@echo "$(GREEN)Documentation: docs/html/index.html$(RESET)"

docs-serve: docs ## Serve documentation locally
	@echo "$(GREEN)Serving documentation at http://localhost:8000$(RESET)"
	cd docs/html && python3 -m http.server 8000

##@ Development

dev-setup: ## Set up development environment
	@echo "$(GREEN)Setting up development environment...$(RESET)"
	./scripts/dev/setup_dev_environment.sh

dev-watch: ## Watch for changes and rebuild
	@echo "$(GREEN)Watching for changes...$(RESET)"
	while true; do \
		inotifywait -r -e modify src include; \
		make build; \
	done

##@ Diagnostics

info: ## Display build system information
	@echo "$(GREEN)Build System Information$(RESET)"
	@echo "  CMake:          $$(cmake --version | head -n1)"
	@echo "  Compiler:       $$(c++ --version | head -n1)"
	@echo "  Build Dir:      $(BUILD_DIR)"
	@echo "  Build Type:     $(BUILD_TYPE)"
	@echo "  Install Prefix: $(INSTALL_PREFIX)"
	@echo "  Generator:      $(CMAKE_GENERATOR)"

version: ## Display project version
	@echo "$(GREEN)2D Truss Analysis v3.0.0$(RESET)"

.PHONY: configure build build-debug build-release rebuild
.PHONY: test test-verbose test-unit test-integration test-coverage
.PHONY: lint format format-check cppcheck
.PHONY: install uninstall install-local
.PHONY: docker-build docker-run docker-dev docker-test
.PHONY: clean clean-all docs docs-serve
.PHONY: dev-setup dev-watch info version
```

---

## 3. Refactored CMakeLists.txt

### 3.1 Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)

# Project metadata
project(TrussAnalysis
    VERSION 3.0.0
    DESCRIPTION "Professional 2D Truss Analysis Tool"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(BUILD_TESTING "Build tests" ON)
option(BUILD_GUI "Build GUI application" ON)
option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_LTO "Enable Link Time Optimization" OFF)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(StaticAnalysis)
include(Sanitizers)
include(InstallRules)

# Find dependencies
find_package(Eigen3 3.4 REQUIRED NO_MODULE)

if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif()

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    enable_testing()
    include(GoogleTest)
endif()

# Subdirectories
add_subdirectory(src/core)
add_subdirectory(src/cli)

if(BUILD_GUI)
    add_subdirectory(src/gui)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Installation
include(GNUInstallDirs)
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/config/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
)

# CPack configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Your Name")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
include(CPack)
```

### 3.2 src/core/CMakeLists.txt

```cmake
# Core library
add_library(TrussCore STATIC)

target_sources(TrussCore
    PRIVATE
        AnalysisEngine.cpp
        Truss.cpp
        Node.cpp
        Member.cpp
        ResultsExporter.cpp
        Logger.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_SOURCE_DIR}/include
        FILES
            ${CMAKE_SOURCE_DIR}/include/truss/AnalysisEngine.hpp
            ${CMAKE_SOURCE_DIR}/include/truss/Truss.hpp
            ${CMAKE_SOURCE_DIR}/include/truss/Node.hpp
            ${CMAKE_SOURCE_DIR}/include/truss/Member.hpp
            ${CMAKE_SOURCE_DIR}/include/truss/ResultsExporter.hpp
            ${CMAKE_SOURCE_DIR}/include/truss/Logger.hpp
            ${CMAKE_SOURCE_DIR}/include/truss/Types.hpp
)

target_include_directories(TrussCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(TrussCore
    PUBLIC
        Eigen3::Eigen
)

# Compiler warnings
if(ENABLE_WARNINGS)
    target_enable_warnings(TrussCore)
endif()

# Sanitizers
if(ENABLE_SANITIZERS)
    target_enable_sanitizers(TrussCore)
endif()

# Link Time Optimization
if(ENABLE_LTO)
    set_target_properties(TrussCore PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()

# Installation
install(TARGETS TrussCore
    EXPORT TrussAnalysisTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS
)
```

### 3.3 src/cli/CMakeLists.txt

```cmake
# CLI executable
add_executable(TrussAnalyze
    main.cpp
    CLIParser.cpp
    ConsoleUI.cpp
)

target_link_libraries(TrussAnalyze
    PRIVATE
        TrussCore
)

# Compiler warnings
if(ENABLE_WARNINGS)
    target_enable_warnings(TrussAnalyze)
endif()

# Installation
install(TARGETS TrussAnalyze
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
```

### 3.4 tests/CMakeLists.txt

```cmake
# Test configuration
include(GoogleTest)

# Helper function to add tests
function(add_truss_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})

    target_link_libraries(${TEST_NAME}
        PRIVATE
            TrussCore
            GTest::gtest_main
            GTest::gmock
    )

    if(ENABLE_COVERAGE)
        target_compile_options(${TEST_NAME} PRIVATE --coverage)
        target_link_options(${TEST_NAME} PRIVATE --coverage)
    endif()

    gtest_discover_tests(${TEST_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        PROPERTIES
            LABELS "unit"
    )
endfunction()

# Unit tests
add_subdirectory(unit)
add_subdirectory(integration)
add_subdirectory(system)
```

---

## 4. CMake Modules

### 4.1 cmake/CompilerWarnings.cmake

```cmake
# CompilerWarnings.cmake - Compiler warning configuration

function(target_enable_warnings target_name)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wconversion
            -Wsign-conversion
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )

        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${target_name} PRIVATE
                -Wmisleading-indentation
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wnull-dereference
            )
        endif()

        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${target_name} PRIVATE
                -Wmost
                -Wextra-semi
                -Wdocumentation
            )
        endif()
    elseif(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /permissive-
            /w14242
            /w14254
            /w14263
            /w14265
            /w14287
            /we4289
            /w14296
            /w14311
            /w14545
            /w14546
            /w14547
            /w14549
            /w14555
            /w14619
            /w14640
            /w14826
            /w14905
            /w14906
            /w14928
        )
    endif()
endfunction()
```

### 4.2 cmake/StaticAnalysis.cmake

```cmake
# StaticAnalysis.cmake - Static analysis tools configuration

option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck" OFF)
option(ENABLE_IWYU "Enable include-what-you-use" OFF)

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY clang-tidy)
    if(CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY};
            -header-filter=.*;
            -checks=*;
        )
        message(STATUS "clang-tidy enabled")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
            ${CPPCHECK};
            --enable=all;
            --suppress=missingIncludeSystem;
            --inline-suppr;
        )
        message(STATUS "cppcheck enabled")
    else()
        message(WARNING "cppcheck requested but not found")
    endif()
endif()

if(ENABLE_IWYU)
    find_program(IWYU include-what-you-use)
    if(IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU})
        message(STATUS "include-what-you-use enabled")
    else()
        message(WARNING "include-what-you-use requested but not found")
    endif()
endif()
```

### 4.3 cmake/Sanitizers.cmake

```cmake
# Sanitizers.cmake - Sanitizer configuration

option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

function(target_enable_sanitizers target_name)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        if(ENABLE_ASAN)
            target_compile_options(${target_name} PRIVATE -fsanitize=address)
            target_link_options(${target_name} PRIVATE -fsanitize=address)
            message(STATUS "Address Sanitizer enabled for ${target_name}")
        endif()

        if(ENABLE_UBSAN)
            target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
            target_link_options(${target_name} PRIVATE -fsanitize=undefined)
            message(STATUS "UB Sanitizer enabled for ${target_name}")
        endif()

        if(ENABLE_TSAN)
            target_compile_options(${target_name} PRIVATE -fsanitize=thread)
            target_link_options(${target_name} PRIVATE -fsanitize=thread)
            message(STATUS "Thread Sanitizer enabled for ${target_name}")
        endif()
    endif()
endfunction()
```

### 4.4 cmake/InstallRules.cmake

```cmake
# InstallRules.cmake - Installation configuration

include(GNUInstallDirs)

# Install headers
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/truss
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Install CMake config files for find_package support
install(EXPORT TrussAnalysisTargets
    FILE TrussAnalysisTargets.cmake
    NAMESPACE TrussAnalysis::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TrussAnalysis
)

# Create config version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TrussAnalysisConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/TrussAnalysisConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TrussAnalysisConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TrussAnalysis
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TrussAnalysisConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TrussAnalysisConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TrussAnalysis
)
```

---

## 5. Build Configurations

### 5.1 .clang-format

```yaml
---
Language: Cpp
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 100
PointerAlignment: Left
ReferenceAlignment: Left
NamespaceIndentation: None
AlignConsecutiveAssignments: true
AlignConsecutiveDeclarations: true
AlignTrailingComments: true
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: Never
AllowShortLoopsOnASingleLine: false
BreakBeforeBraces: Attach
IncludeBlocks: Regroup
SortIncludes: true
...
```

### 5.2 .clang-tidy

```yaml
---
Checks: >
  -*,
  bugprone-*,
  cppcoreguidelines-*,
  modernize-*,
  performance-*,
  readability-*,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers,
  -cppcoreguidelines-avoid-magic-numbers

WarningsAsErrors: "*"

CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.FunctionCase
    value: camelCase
  - key: readability-identifier-naming.VariableCase
    value: camelCase
  - key: readability-identifier-naming.ConstantCase
    value: UPPER_CASE
  - key: readability-identifier-naming.EnumCase
    value: CamelCase
  - key: readability-identifier-naming.NamespaceCase
    value: lower_case
...
```

---

## 6. CI/CD Integration

### 6.1 GitHub Actions Workflow

```yaml
# .github/workflows/build-and-test.yml
name: Build and Test

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main]

jobs:
  build-linux:
    runs-on: ubuntu-22.04
    strategy:
      matrix:
        build_type: [Debug, Release]
        compiler:
          - { cc: gcc-12, cxx: g++-12 }
          - { cc: clang-15, cxx: clang++-15 }

    steps:
      - uses: actions/checkout@v3

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            cmake \
            ninja-build \
            libeigen3-dev \
            libgtest-dev \
            ${{ matrix.compiler.cc }} \
            ${{ matrix.compiler.cxx }}

      - name: Configure
        env:
          CC: ${{ matrix.compiler.cc }}
          CXX: ${{ matrix.compiler.cxx }}
        run: |
          cmake -B build -G Ninja \
            -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} \
            -DBUILD_TESTING=ON \
            -DENABLE_WARNINGS=ON

      - name: Build
        run: cmake --build build -j$(nproc)

      - name: Test
        run: cd build && ctest --output-on-failure

  code-quality:
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v3

      - name: Install tools
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-format clang-tidy cppcheck

      - name: Check formatting
        run: make format-check

      - name: Run clang-tidy
        run: make lint

      - name: Run cppcheck
        run: make cppcheck

  coverage:
    runs-on: ubuntu-22.04
    steps:
      - uses: actions/checkout@v3

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            cmake ninja-build libeigen3-dev libgtest-dev \
            lcov

      - name: Generate coverage
        run: make test-coverage

      - name: Upload to Codecov
        uses: codecov/codecov-action@v3
        with:
          files: build/coverage.info
```

---

## 7. Implementation Checklist

### Phase 1: Makefile Creation

- [ ] Create root Makefile with all targets
- [ ] Test `make build`
- [ ] Test `make test`
- [ ] Test `make install`
- [ ] Test `make docker-build`
- [ ] Verify `make help` output

### Phase 2: CMake Refactoring

- [ ] Refactor root CMakeLists.txt
- [ ] Create src/core/CMakeLists.txt
- [ ] Create src/cli/CMakeLists.txt
- [ ] Create src/gui/CMakeLists.txt
- [ ] Create tests/CMakeLists.txt
- [ ] Remove all macOS-specific code
- [ ] Verify builds on Linux

### Phase 3: CMake Modules

- [ ] Create cmake/CompilerWarnings.cmake
- [ ] Create cmake/StaticAnalysis.cmake
- [ ] Create cmake/Sanitizers.cmake
- [ ] Create cmake/InstallRules.cmake
- [ ] Test each module independently

### Phase 4: Configuration Files

- [ ] Create .clang-format
- [ ] Create .clang-tidy
- [ ] Test formatting: `make format`
- [ ] Test linting: `make lint`

### Phase 5: CI/CD

- [ ] Create .github/workflows/build-and-test.yml
- [ ] Test GitHub Actions pipeline
- [ ] Verify all build configurations
- [ ] Verify code quality checks

---

## 8. Success Criteria

✅ **Makefile provides all essential targets**  
✅ **CMake builds cleanly on Ubuntu 22.04**  
✅ **No macOS-specific code remains**  
✅ **All tests pass**  
✅ **Code formatting enforced**  
✅ **Static analysis integrated**  
✅ **CI/CD pipeline operational**  
✅ **Installation works correctly**

---

**Document Status:** Ready for Implementation  
**Next:** Begin Phase 0 implementation (Foundation & Cleanup)
