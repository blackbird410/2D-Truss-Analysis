# ==============================================================================
# 2D Truss Analysis - Production Makefile
# ==============================================================================
#
# A professional build system wrapper for CMake-based C++ projects.
# Provides intuitive targets for development, testing, and deployment.
#
# NOTE: You may see a warning about overriding the 'build' target. This is
#       harmless and occurs because CMake generates its own Makefile in build/
#       with its own 'build' target. The top-level Makefile's target takes
#       precedence, which is the correct behavior. To suppress: make -w help
#
# Prerequisites:
#   - CMake 3.20+
#   - C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
#   - Qt6, Eigen3, nlohmann_json, tinyxml2, GoogleTest
#
# Quick Start:
#   make              # Show help
#   make build        # Build release version
#   make test         # Run all tests
#   make coverage     # Generate coverage report
#
# ==============================================================================

# Enforce consistent behavior
SHELL := /bin/bash
.SHELLFLAGS := -e -u -o pipefail -c
.DELETE_ON_ERROR:
.SUFFIXES:

# Prevent Make from treating directories as targets
MAKEFLAGS += --no-builtin-rules --no-print-directory
.SUFFIXES:

# Tell Make to ignore CMake's generated Makefiles in build directories
# This prevents warnings about overriding the 'build' target
MAKEFLAGS += --include-dir=.

# Default target
.DEFAULT_GOAL := help

# ==============================================================================
# Configuration
# ==============================================================================

# Project metadata
PROJECT_NAME := TrussAnalysis
VERSION := 3.0.0

# Build directories
BUILD_DIR := build
BUILD_DEBUG_DIR := build_debug
BUILD_COVERAGE_DIR := build_coverage

# Detect number of CPU cores for parallel builds
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# CMake generator (prefer Ninja if available, fallback to Unix Makefiles)
CMAKE_GENERATOR := $(shell command -v ninja >/dev/null 2>&1 && echo "Ninja" || echo "Unix Makefiles")

# CMake build tool
ifeq ($(CMAKE_GENERATOR),Ninja)
    BUILD_TOOL := ninja
else
    BUILD_TOOL := $(MAKE) -j$(NPROC)
endif

# Compiler detection
CXX ?= $(shell command -v clang++ 2>/dev/null || command -v g++ 2>/dev/null || echo c++)

# Coverage tools
LCOV := $(shell command -v lcov 2>/dev/null)
GENHTML := $(shell command -v genhtml 2>/dev/null)

# Formatting tools
CLANG_FORMAT := $(shell command -v clang-format 2>/dev/null)

# Static analysis tools
CLANG_TIDY := $(shell command -v clang-tidy 2>/dev/null)
CPPCHECK := $(shell command -v cppcheck 2>/dev/null)

# Output styling
BOLD := \033[1m
RESET := \033[0m
GREEN := \033[32m
YELLOW := \033[33m
BLUE := \033[36m

# ==============================================================================
# Help Target
# ==============================================================================

.PHONY: help
help: ## Show this help message
	@echo -e "$(BOLD)$(PROJECT_NAME) v$(VERSION) - Production Build System$(RESET)"
	@echo ""
	@echo -e "$(BOLD)Usage:$(RESET) make [target]"
	@echo ""
	@echo -e "$(BOLD)Build Targets:$(RESET)"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; /^[a-zA-Z_-]+:.*?## .*$$/ {printf "  $(BLUE)%-20s$(RESET) %s\n", $$1, $$2}' | \
		grep -E "build|clean|configure"
	@echo ""
	@echo -e "$(BOLD)Test Targets:$(RESET)"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; /^[a-zA-Z_-]+:.*?## .*$$/ {printf "  $(BLUE)%-20s$(RESET) %s\n", $$1, $$2}' | \
		grep -E "test|coverage"
	@echo ""
	@echo -e "$(BOLD)Code Quality:$(RESET)"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; /^[a-zA-Z_-]+:.*?## .*$$/ {printf "  $(BLUE)%-20s$(RESET) %s\n", $$1, $$2}' | \
		grep -E "format|lint|static"
	@echo ""
	@echo -e "$(BOLD)Development:$(RESET)"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; /^[a-zA-Z_-]+:.*?## .*$$/ {printf "  $(BLUE)%-20s$(RESET) %s\n", $$1, $$2}' | \
		grep -E "run|debug|install|info"
	@echo ""
	@echo -e "$(BOLD)Examples:$(RESET)"
	@echo "  make build test        # Build and test (release mode)"
	@echo "  make debug             # Build debug version"
	@echo "  make coverage          # Generate coverage report"
	@echo "  make clean-all         # Clean all build artifacts"
	@echo ""
	@echo -e "$(BOLD)Environment:$(RESET)"
	@echo "  CXX:           $(CXX)"
	@echo "  CPU Cores:     $(NPROC)"
	@echo "  CMake Gen:     $(CMAKE_GENERATOR)"
	@echo ""
	@echo -e "$(BOLD)Note:$(RESET) Warnings about 'build' target override are harmless (CMake-generated Makefile)"
	@echo ""

# ==============================================================================
# Build Targets
# ==============================================================================

.PHONY: all
all: build ## Build everything (alias for 'build')

.PHONY: build
build: configure-release ## Build release version (optimized)
	@echo -e "$(BOLD)Building release version...$(RESET)"
	@cmake --build $(BUILD_DIR) --parallel $(NPROC)
	@echo -e "$(GREEN)✓ Build complete$(RESET)"

.PHONY: debug
debug: configure-debug ## Build debug version (with symbols)
	@echo -e "$(BOLD)Building debug version...$(RESET)"
	@cmake --build $(BUILD_DEBUG_DIR) --parallel $(NPROC)
	@echo -e "$(GREEN)✓ Debug build complete$(RESET)"

.PHONY: configure-release
configure-release:
	@mkdir -p $(BUILD_DIR)
	@if [ ! -f $(BUILD_DIR)/CMakeCache.txt ]; then \
		echo -e "$(BOLD)Configuring release build...$(RESET)"; \
		cd $(BUILD_DIR) && cmake -G "$(CMAKE_GENERATOR)" \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_CXX_COMPILER=$(CXX) \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
			..; \
	fi

.PHONY: configure-debug
configure-debug:
	@mkdir -p $(BUILD_DEBUG_DIR)
	@if [ ! -f $(BUILD_DEBUG_DIR)/CMakeCache.txt ]; then \
		echo -e "$(BOLD)Configuring debug build...$(RESET)"; \
		cd $(BUILD_DEBUG_DIR) && cmake -G "$(CMAKE_GENERATOR)" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_CXX_COMPILER=$(CXX) \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
			..; \
	fi

.PHONY: configure-coverage
configure-coverage:
	@mkdir -p $(BUILD_COVERAGE_DIR)
	@if [ ! -f $(BUILD_COVERAGE_DIR)/CMakeCache.txt ]; then \
		echo -e "$(BOLD)Configuring coverage build...$(RESET)"; \
		cd $(BUILD_COVERAGE_DIR) && cmake -G "$(CMAKE_GENERATOR)" \
			-DCMAKE_BUILD_TYPE=Debug \
			-DENABLE_COVERAGE=ON \
			-DCMAKE_CXX_COMPILER=$(CXX) \
			..; \
	fi

.PHONY: rebuild
rebuild: clean build ## Clean and rebuild release version

.PHONY: rebuild-debug
rebuild-debug: clean-debug debug ## Clean and rebuild debug version

# ==============================================================================
# Test Targets
# ==============================================================================

.PHONY: test
test: build ## Run all tests (release build)
	@echo -e "$(BOLD)Running all tests...$(RESET)"
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel $(NPROC)
	@echo -e "$(GREEN)✓ All tests passed$(RESET)"

.PHONY: test-verbose
test-verbose: build ## Run tests with verbose output
	@echo -e "$(BOLD)Running tests (verbose)...$(RESET)"
	@cd $(BUILD_DIR) && ctest --verbose --parallel $(NPROC)

.PHONY: test-debug
test-debug: debug ## Run tests in debug mode
	@echo -e "$(BOLD)Running tests (debug build)...$(RESET)"
	@cd $(BUILD_DEBUG_DIR) && ctest --output-on-failure --parallel $(NPROC)

.PHONY: test-unit
test-unit: build ## Run unit tests only
	@echo -e "$(BOLD)Running unit tests...$(RESET)"
	@cd $(BUILD_DIR) && ctest -R UnitTests --output-on-failure

.PHONY: test-integration
test-integration: build ## Run integration tests only
	@echo -e "$(BOLD)Running integration tests...$(RESET)"
	@cd $(BUILD_DIR) && ctest -R IntegrationTests --output-on-failure

.PHONY: test-gui
test-gui: build ## Run GUI integration tests
	@echo -e "$(BOLD)Running GUI integration tests...$(RESET)"
	@cd $(BUILD_DIR) && ctest -R GUIIntegrationTests --output-on-failure

# ==============================================================================
# Coverage Targets
# ==============================================================================

.PHONY: coverage
coverage: configure-coverage ## Generate coverage report
	@echo -e "$(BOLD)Building with coverage instrumentation...$(RESET)"
	@cmake --build $(BUILD_COVERAGE_DIR) --parallel $(NPROC)
	@echo -e "$(BOLD)Running tests with coverage...$(RESET)"
	@cd $(BUILD_COVERAGE_DIR) && ctest --output-on-failure --parallel $(NPROC)
	@if [ -z "$(LCOV)" ] || [ -z "$(GENHTML)" ]; then \
		echo -e "$(YELLOW)⚠ lcov/genhtml not found, skipping report generation$(RESET)"; \
		echo "  Install: brew install lcov (macOS) or apt install lcov (Linux)"; \
		exit 1; \
	fi
	@echo -e "$(BOLD)Generating coverage report...$(RESET)"
	@cd $(BUILD_COVERAGE_DIR) && $(MAKE) coverage
	@echo -e "$(GREEN)✓ Coverage report: $(BUILD_COVERAGE_DIR)/coverage_html/index.html$(RESET)"

.PHONY: coverage-open
coverage-open: coverage ## Generate and open coverage report in browser
	@if [ -f $(BUILD_COVERAGE_DIR)/coverage_html/index.html ]; then \
		open $(BUILD_COVERAGE_DIR)/coverage_html/index.html 2>/dev/null || \
		xdg-open $(BUILD_COVERAGE_DIR)/coverage_html/index.html 2>/dev/null || \
		echo -e "$(YELLOW)Coverage report: $(BUILD_COVERAGE_DIR)/coverage_html/index.html$(RESET)"; \
	else \
		echo -e "$(YELLOW)⚠ Coverage report not found$(RESET)"; \
	fi

# ==============================================================================
# Code Quality Targets
# ==============================================================================

.PHONY: format
format: ## Format C++ code with clang-format
	@if [ -z "$(CLANG_FORMAT)" ]; then \
		echo -e "$(YELLOW)⚠ clang-format not found$(RESET)"; \
		echo "  Install: brew install llvm (macOS) or apt install clang-format (Linux)"; \
		exit 1; \
	fi
	@echo -e "$(BOLD)Formatting C++ code...$(RESET)"
	@find src tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
		-not -path "*/build*/*" \
		-exec $(CLANG_FORMAT) -i -style=file {} +
	@echo -e "$(GREEN)✓ Code formatted$(RESET)"

.PHONY: format-check
format-check: ## Check if code needs formatting (CI-friendly)
	@if [ -z "$(CLANG_FORMAT)" ]; then \
		echo -e "$(YELLOW)⚠ clang-format not found, skipping check$(RESET)"; \
		exit 0; \
	fi
	@echo -e "$(BOLD)Checking code formatting...$(RESET)"
	@if ! find src tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
		-not -path "*/build*/*" \
		-exec $(CLANG_FORMAT) --dry-run -Werror -style=file {} + 2>/dev/null; then \
		echo -e "$(YELLOW)❌ Code needs formatting. Run 'make format'$(RESET)"; \
		exit 1; \
	fi
	@echo -e "$(GREEN)✓ Code is properly formatted$(RESET)"

.PHONY: lint
lint: ## Run clang-tidy static analysis
	@if [ -z "$(CLANG_TIDY)" ]; then \
		echo -e "$(YELLOW)⚠ clang-tidy not found$(RESET)"; \
		echo "  Install: brew install llvm (macOS) or apt install clang-tidy (Linux)"; \
		exit 1; \
	fi
	@if [ ! -f $(BUILD_DIR)/compile_commands.json ]; then \
		echo -e "$(YELLOW)⚠ compile_commands.json not found. Run 'make build' first$(RESET)"; \
		exit 1; \
	fi
	@echo -e "$(BOLD)Running clang-tidy...$(RESET)"
	@find src -type f \( -name "*.cpp" \) \
		-not -path "*/build*/*" \
		-exec $(CLANG_TIDY) -p $(BUILD_DIR) {} +
	@echo -e "$(GREEN)✓ Static analysis complete$(RESET)"

.PHONY: static-analysis
static-analysis: ## Run cppcheck static analysis
	@if [ -z "$(CPPCHECK)" ]; then \
		echo -e "$(YELLOW)⚠ cppcheck not found$(RESET)"; \
		echo "  Install: brew install cppcheck (macOS) or apt install cppcheck (Linux)"; \
		exit 1; \
	fi
	@echo -e "$(BOLD)Running cppcheck...$(RESET)"
	@$(CPPCHECK) --enable=all --inconclusive --std=c++20 \
		--suppress=missingIncludeSystem \
		-I src \
		src/ 2>&1 | tee cppcheck-report.txt
	@echo -e "$(GREEN)✓ Static analysis complete (see cppcheck-report.txt)$(RESET)"

# ==============================================================================
# Development Targets
# ==============================================================================

.PHONY: run-cli
run-cli: build ## Run CLI application (example command)
	@echo -e "$(BOLD)Running CLI application...$(RESET)"
	@$(BUILD_DIR)/TrussAnalysisCLI --help

.PHONY: run-gui
run-gui: build ## Run GUI application
	@echo -e "$(BOLD)Running GUI application...$(RESET)"
	@$(BUILD_DIR)/TrussAnalysisGUI

.PHONY: install
install: build ## Install to system (requires sudo)
	@echo -e "$(BOLD)Installing $(PROJECT_NAME)...$(RESET)"
	@cd $(BUILD_DIR) && sudo cmake --install .
	@echo -e "$(GREEN)✓ Installation complete$(RESET)"

.PHONY: info
info: ## Show build system information
	@echo -e "$(BOLD)Build System Information$(RESET)"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo -e "$(BOLD)Project:$(RESET)       $(PROJECT_NAME) v$(VERSION)"
	@echo -e "$(BOLD)Compiler:$(RESET)      $(CXX)"
	@echo -e "$(BOLD)CPU Cores:$(RESET)     $(NPROC)"
	@echo -e "$(BOLD)CMake Gen:$(RESET)     $(CMAKE_GENERATOR)"
	@echo -e "$(BOLD)Build Tool:$(RESET)    $(BUILD_TOOL)"
	@echo ""
	@echo -e "$(BOLD)Build Directories:$(RESET)"
	@echo "  Release:       $(BUILD_DIR)"
	@echo "  Debug:         $(BUILD_DEBUG_DIR)"
	@echo "  Coverage:      $(BUILD_COVERAGE_DIR)"
	@echo ""
	@echo -e "$(BOLD)Available Tools:$(RESET)"
	@echo "  lcov:          $(if $(LCOV),✓ $(LCOV),✗ not found)"
	@echo "  genhtml:       $(if $(GENHTML),✓ $(GENHTML),✗ not found)"
	@echo "  clang-format:  $(if $(CLANG_FORMAT),✓ $(CLANG_FORMAT),✗ not found)"
	@echo "  clang-tidy:    $(if $(CLANG_TIDY),✓ $(CLANG_TIDY),✗ not found)"
	@echo "  cppcheck:      $(if $(CPPCHECK),✓ $(CPPCHECK),✗ not found)"
	@echo ""
	@if [ -d $(BUILD_DIR) ]; then \
		echo -e "$(BOLD)Build Status (Release):$(RESET)"; \
		if [ -f $(BUILD_DIR)/TrussAnalysisCLI ]; then echo "  CLI:           ✓ built"; else echo "  CLI:           ✗ not built"; fi; \
		if [ -f $(BUILD_DIR)/TrussAnalysisGUI ]; then echo "  GUI:           ✓ built"; else echo "  GUI:           ✗ not built"; fi; \
		if [ -f $(BUILD_DIR)/libTrussCore.a ]; then echo "  TrussCore:     ✓ built"; else echo "  TrussCore:     ✗ not built"; fi; \
		if [ -f $(BUILD_DIR)/libTrussApplication.a ]; then echo "  Application:   ✓ built"; else echo "  Application:   ✗ not built"; fi; \
	else \
		echo -e "$(YELLOW)Release build not configured$(RESET)"; \
	fi

# ==============================================================================
# Clean Targets
# ==============================================================================

.PHONY: clean
clean: ## Clean release build artifacts
	@echo -e "$(BOLD)Cleaning release build...$(RESET)"
	@rm -rf $(BUILD_DIR)
	@echo -e "$(GREEN)✓ Release build cleaned$(RESET)"

.PHONY: clean-debug
clean-debug: ## Clean debug build artifacts
	@echo -e "$(BOLD)Cleaning debug build...$(RESET)"
	@rm -rf $(BUILD_DEBUG_DIR)
	@echo -e "$(GREEN)✓ Debug build cleaned$(RESET)"

.PHONY: clean-coverage
clean-coverage: ## Clean coverage build artifacts
	@echo -e "$(BOLD)Cleaning coverage build...$(RESET)"
	@rm -rf $(BUILD_COVERAGE_DIR)
	@echo -e "$(GREEN)✓ Coverage build cleaned$(RESET)"

.PHONY: clean-all
clean-all: clean clean-debug clean-coverage ## Clean all build artifacts
	@echo -e "$(BOLD)Cleaning additional artifacts...$(RESET)"
	@rm -f cppcheck-report.txt
	@rm -rf bin/ lib/
	@echo -e "$(GREEN)✓ All build artifacts cleaned$(RESET)"

.PHONY: distclean
distclean: clean-all ## Deep clean (including CMake caches)
	@echo -e "$(BOLD)Performing deep clean...$(RESET)"
	@find . -type f -name "CMakeCache.txt" -delete
	@find . -type d -name "CMakeFiles" -exec rm -rf {} + 2>/dev/null || true
	@echo -e "$(GREEN)✓ Deep clean complete$(RESET)"

# ==============================================================================
# CI/CD Targets
# ==============================================================================

.PHONY: ci
ci: build test format-check ## CI pipeline (build + test + format check)
	@echo -e "$(GREEN)✓ CI pipeline passed$(RESET)"

.PHONY: ci-full
ci-full: ci coverage ## Full CI pipeline (build + test + coverage)
	@echo -e "$(GREEN)✓ Full CI pipeline passed$(RESET)"

# ==============================================================================
# Phony Target Declaration
# ==============================================================================
# Note: Directory creation is handled inline in configure-* targets

.PHONY: all build debug configure-release configure-debug configure-coverage \
        rebuild rebuild-debug test test-verbose test-debug test-unit \
        test-integration test-gui coverage coverage-open format format-check \
        lint static-analysis run-cli run-gui install info clean clean-debug \
        clean-coverage clean-all distclean ci ci-full help
