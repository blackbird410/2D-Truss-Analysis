# ==============================================================================
# 2D Truss Analysis - Production Makefile
# ==============================================================================
#
# A professional build system wrapper for CMake-based C++ projects.
# Delegates to helper scripts in ./scripts/ for build, test, and code quality tasks.
#
# This Makefile serves as a convenient interface for common development tasks.
# For more advanced usage, invoke the scripts directly:
#   - ./scripts/build.sh [options]
#   - ./scripts/test.sh [test-type]
#   - ./scripts/format-code.sh
#   - ./scripts/run-static-analysis.sh
#   - ./scripts/generate-docs.sh
#
# Quick Start:
#   make              # Show help
#   make build        # Build release version
#   make test         # Run all tests
#   make format       # Format code
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

# Script paths
SCRIPT_DIR := scripts
BUILD_SCRIPT := $(SCRIPT_DIR)/build.sh
TEST_SCRIPT := $(SCRIPT_DIR)/test.sh
FORMAT_SCRIPT := $(SCRIPT_DIR)/format-code.sh
ANALYSIS_SCRIPT := $(SCRIPT_DIR)/run-static-analysis.sh
DOCS_SCRIPT := $(SCRIPT_DIR)/generate-docs.sh
COVERAGE_SCRIPT := $(SCRIPT_DIR)/generate_coverage.sh
INSTALL_UBUNTU_SCRIPT := $(SCRIPT_DIR)/install-deps-ubuntu.sh
INSTALL_FEDORA_SCRIPT := $(SCRIPT_DIR)/install-deps-fedora.sh

# Coverage tools
LCOV := $(shell command -v lcov 2>/dev/null)
GENHTML := $(shell command -v genhtml 2>/dev/null)

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
	@echo -e "  $(BLUE)build$(RESET)           Build release version (uses ./scripts/build.sh)"
	@echo -e "  $(BLUE)debug$(RESET)           Build debug version"
	@echo -e "  $(BLUE)rebuild$(RESET)         Clean and rebuild release version"
	@echo -e "  $(BLUE)rebuild-debug$(RESET)   Clean and rebuild debug version"
	@echo -e "  $(BLUE)clean$(RESET)           Clean release build artifacts"
	@echo -e "  $(BLUE)clean-debug$(RESET)     Clean debug build artifacts"
	@echo -e "  $(BLUE)clean-coverage$(RESET)  Clean coverage build artifacts"
	@echo -e "  $(BLUE)clean-all$(RESET)       Clean all build artifacts"
	@echo -e "  $(BLUE)distclean$(RESET)       Deep clean (including CMake caches)"
	@echo ""
	@echo -e "$(BOLD)Test Targets:$(RESET)"
	@echo -e "  $(BLUE)test$(RESET)            Run all tests (uses ./scripts/test.sh)"
	@echo -e "  $(BLUE)test-unit$(RESET)       Run unit tests only"
	@echo -e "  $(BLUE)test-integration$(RESET) Run integration tests"
	@echo -e "  $(BLUE)test-gui$(RESET)        Run GUI integration tests"
	@echo -e "  $(BLUE)test-verbose$(RESET)    Run tests with verbose output"
	@echo -e "  $(BLUE)test-debug$(RESET)      Run tests in debug mode"
	@echo ""
	@echo -e "$(BOLD)Code Quality:$(RESET)"
	@echo -e "  $(BLUE)format$(RESET)          Format C++ code (uses ./scripts/format-code.sh)"
	@echo -e "  $(BLUE)format-check$(RESET)    Check if code needs formatting (CI-friendly)"
	@echo -e "  $(BLUE)lint$(RESET)            Run clang-tidy static analysis"
	@echo -e "  $(BLUE)static-analysis$(RESET) Run cppcheck analysis (uses ./scripts/run-static-analysis.sh)"
	@echo -e "  $(BLUE)coverage$(RESET)        Generate code coverage report"
	@echo -e "  $(BLUE)coverage-open$(RESET)   Generate and open coverage in browser"
	@echo ""
	@echo -e "$(BOLD)Documentation:$(RESET)"
	@echo -e "  $(BLUE)docs$(RESET)            Generate API documentation (uses ./scripts/generate-docs.sh)"
	@echo ""
	@echo -e "$(BOLD)Development:$(RESET)"
	@echo -e "  $(BLUE)run-cli$(RESET)         Run CLI application"
	@echo -e "  $(BLUE)run-gui$(RESET)         Run GUI application"
	@echo -e "  $(BLUE)install$(RESET)         Install to system (requires sudo)"
	@echo -e "  $(BLUE)info$(RESET)            Show build system information"
	@echo ""
	@echo -e "$(BOLD)Dependencies:$(RESET)"
	@echo -e "  $(BLUE)install-ubuntu$(RESET)  Install dependencies (Ubuntu/Debian)"
	@echo -e "  $(BLUE)install-fedora$(RESET)  Install dependencies (Fedora/RHEL)"
	@echo ""
	@echo -e "$(BOLD)Docker/Containers:$(RESET)"
	@echo -e "  $(BLUE)docker-build$(RESET)    Build production Docker image"
	@echo -e "  $(BLUE)docker-build-dev$(RESET) Build development Docker image"
	@echo -e "  $(BLUE)docker-run$(RESET)      Run analysis in Docker container"
	@echo -e "  $(BLUE)docker-dev$(RESET)      Start interactive development container"
	@echo -e "  $(BLUE)docker-shell$(RESET)    Open shell in running container"
	@echo -e "  $(BLUE)docker-test$(RESET)     Test Docker container"
	@echo -e "  $(BLUE)docker-clean$(RESET)    Remove Docker images and containers"
	@echo ""
	@echo -e "$(BOLD)CI/CD:$(RESET)"
	@echo -e "  $(BLUE)ci$(RESET)              Run CI pipeline (build + test + format-check)"
	@echo -e "  $(BLUE)ci-full$(RESET)         Run full CI pipeline (+ coverage)"
	@echo ""
	@echo -e "$(BOLD)Examples:$(RESET)"
	@echo "  make build test        # Build and test (release mode)"
	@echo "  make debug             # Build debug version"
	@echo "  make format            # Format code"
	@echo "  make coverage          # Generate coverage report"
	@echo "  make docker-build      # Build Docker container"
	@echo "  make clean-all         # Clean all artifacts"
	@echo ""

# ==============================================================================
# Build Targets
# ==============================================================================

.PHONY: all
all: build ## Build everything (alias for 'build')

.PHONY: build
build: ## Build release version (optimized, uses ./scripts/build.sh)
	@echo -e "$(BOLD)Building release version...$(RESET)"
	@$(BUILD_SCRIPT) --release -j$(NPROC)
	@echo -e "$(GREEN)✓ Build complete$(RESET)"

.PHONY: debug
debug: ## Build debug version (with symbols)
	@echo -e "$(BOLD)Building debug version...$(RESET)"
	@$(BUILD_SCRIPT) --debug -j$(NPROC)
	@echo -e "$(GREEN)✓ Debug build complete$(RESET)"

.PHONY: rebuild
rebuild: clean build ## Clean and rebuild release version

.PHONY: rebuild-debug
rebuild-debug: clean-debug debug ## Clean and rebuild debug version

# ==============================================================================
# Test Targets
# ==============================================================================

.PHONY: test
test: ## Run all tests (uses ./scripts/test.sh)
	@echo -e "$(BOLD)Running all tests...$(RESET)"
	@if [ ! -d "$(BUILD_DIR)" ] || grep -q "BUILD_TESTING:BOOL=OFF" "$(BUILD_DIR)/CMakeCache.txt" 2>/dev/null; then \
		echo -e "$(YELLOW)Tests not configured. Reconfiguring with BUILD_TESTING=ON...$(RESET)"; \
		$(BUILD_SCRIPT) --clean; \
	fi
	@$(TEST_SCRIPT) all
	@echo -e "$(GREEN)✓ All tests passed$(RESET)"

.PHONY: test-unit
test-unit: ## Run unit tests only
	@echo -e "$(BOLD)Running unit tests...$(RESET)"
	@if [ ! -d "$(BUILD_DIR)" ] || grep -q "BUILD_TESTING:BOOL=OFF" "$(BUILD_DIR)/CMakeCache.txt" 2>/dev/null; then \
		echo -e "$(YELLOW)Tests not configured. Reconfiguring with BUILD_TESTING=ON...$(RESET)"; \
		$(BUILD_SCRIPT) --clean; \
	fi
	@$(TEST_SCRIPT) unit
	@echo -e "$(GREEN)✓ Unit tests passed$(RESET)"

.PHONY: test-integration
test-integration: ## Run integration tests only
	@echo -e "$(BOLD)Running integration tests...$(RESET)"
	@if [ ! -d "$(BUILD_DIR)" ] || grep -q "BUILD_TESTING:BOOL=OFF" "$(BUILD_DIR)/CMakeCache.txt" 2>/dev/null; then \
		echo -e "$(YELLOW)Tests not configured. Reconfiguring with BUILD_TESTING=ON...$(RESET)"; \
		$(BUILD_SCRIPT) --clean; \
	fi
	@$(TEST_SCRIPT) integration
	@echo -e "$(GREEN)✓ Integration tests passed$(RESET)"

.PHONY: test-gui
test-gui: build ## Run GUI integration tests
	@echo -e "$(BOLD)Running GUI integration tests...$(RESET)"
	@$(TEST_SCRIPT) gui
	@echo -e "$(GREEN)✓ GUI tests passed$(RESET)"

.PHONY: test-verbose
test-verbose: build ## Run tests with verbose output
	@echo -e "$(BOLD)Running tests (verbose)...$(RESET)"
	@cd $(BUILD_DIR) && ctest --verbose --parallel $(NPROC)

.PHONY: test-debug
test-debug: debug ## Run tests in debug mode
	@echo -e "$(BOLD)Running tests (debug build)...$(RESET)"
	@cd $(BUILD_DEBUG_DIR) && ctest --output-on-failure --parallel $(NPROC)

# ==============================================================================
# Coverage Targets
# ==============================================================================

.PHONY: coverage
coverage: ## Generate code coverage report
	@echo -e "$(BOLD)Generating coverage report...$(RESET)"
	@if [ ! -x "$(COVERAGE_SCRIPT)" ]; then \
		echo -e "$(YELLOW)⚠ Coverage script not executable$(RESET)"; \
		exit 1; \
	fi
	@$(COVERAGE_SCRIPT)
	@echo -e "$(GREEN)✓ Coverage report generated$(RESET)"

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
format: ## Format C++ code (uses ./scripts/format-code.sh)
	@echo -e "$(BOLD)Formatting C++ code...$(RESET)"
	@$(FORMAT_SCRIPT)
	@echo -e "$(GREEN)✓ Code formatted$(RESET)"

.PHONY: format-check
format-check: ## Check if code needs formatting (CI-friendly)
	@echo -e "$(BOLD)Checking code formatting...$(RESET)"
	@command -v clang-format >/dev/null 2>&1 || \
		(echo -e "$(YELLOW)⚠ clang-format not found$(RESET)"; exit 1)
	@if find src tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
		-not -path "*/build*/*" \
		-exec clang-format --dry-run -Werror -style=file {} + 2>/dev/null; then \
		echo -e "$(GREEN)✓ Code is properly formatted$(RESET)"; \
	else \
		echo -e "$(YELLOW)⚠ Code needs formatting. Run 'make format'$(RESET)"; \
		exit 1; \
	fi

.PHONY: lint
lint: build ## Run clang-tidy static analysis
	@echo -e "$(BOLD)Running clang-tidy...$(RESET)"
	@command -v clang-tidy >/dev/null 2>&1 || \
		(echo -e "$(YELLOW)⚠ clang-tidy not found$(RESET)"; exit 1)
	@if [ ! -f $(BUILD_DIR)/compile_commands.json ]; then \
		echo -e "$(YELLOW)⚠ compile_commands.json not found. Run 'make build' first$(RESET)"; \
		exit 1; \
	fi
	@find src -type f -name "*.cpp" \
		-not -path "*/build*/*" \
		-exec clang-tidy -p $(BUILD_DIR) {} +
	@echo -e "$(GREEN)✓ Static analysis complete$(RESET)"

.PHONY: static-analysis
static-analysis: ## Run cppcheck static analysis (uses ./scripts/run-static-analysis.sh)
	@echo -e "$(BOLD)Running static analysis...$(RESET)"
	@$(ANALYSIS_SCRIPT)
	@echo -e "$(GREEN)✓ Static analysis complete$(RESET)"

# ==============================================================================
# Documentation Targets
# ==============================================================================

.PHONY: docs
docs: ## Generate API documentation (uses ./scripts/generate-docs.sh)
	@echo -e "$(BOLD)Generating API documentation...$(RESET)"
	@$(DOCS_SCRIPT)
	@echo -e "$(GREEN)✓ Documentation generated$(RESET)"

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

.PHONY: install-ubuntu
install-ubuntu: ## Install dependencies (Ubuntu/Debian, uses ./scripts/install-deps-ubuntu.sh)
	@echo -e "$(BOLD)Installing Ubuntu/Debian dependencies...$(RESET)"
	@$(INSTALL_UBUNTU_SCRIPT)

.PHONY: install-fedora
install-fedora: ## Install dependencies (Fedora/RHEL, uses ./scripts/install-deps-fedora.sh)
	@echo -e "$(BOLD)Installing Fedora/RHEL dependencies...$(RESET)"
	@$(INSTALL_FEDORA_SCRIPT)

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
	@echo -e "$(BOLD)Scripts:$(RESET)"
	@echo "  Build:         $(BUILD_SCRIPT)"
	@echo "  Test:          $(TEST_SCRIPT)"
	@echo "  Format:        $(FORMAT_SCRIPT)"
	@echo "  Analysis:      $(ANALYSIS_SCRIPT)"
	@echo "  Docs:          $(DOCS_SCRIPT)"
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
# Docker/Container Targets
# ==============================================================================

# Docker configuration
DOCKER_IMAGE := truss-analysis
DOCKER_TAG := latest
REGISTRY := ghcr.io/blackbird410

.PHONY: docker-build
docker-build: ## Build production Docker image (CLI only)
	@echo -e "$(BOLD)Building production Docker image...$(RESET)"
	@DOCKER_BUILDKIT=1 docker build -t $(DOCKER_IMAGE):$(DOCKER_TAG) -f docker/Dockerfile .
	@echo -e "$(GREEN)✓ Docker image built: $(DOCKER_IMAGE):$(DOCKER_TAG)$(RESET)"

.PHONY: docker-build-dev
docker-build-dev: ## Build development Docker image
	@echo -e "$(BOLD)Building development Docker image...$(RESET)"
	@DOCKER_BUILDKIT=1 docker build -t $(DOCKER_IMAGE):dev -f docker/Dockerfile.dev .
	@echo -e "$(GREEN)✓ Development image built: $(DOCKER_IMAGE):dev$(RESET)"

.PHONY: docker-run
docker-run: ## Run analysis in Docker container
	@echo -e "$(BOLD)Running Docker container...$(RESET)"
	@docker run --rm -it \
		-v $(PWD)/tests/fixtures/sample_data:/data/input:ro \
		-v $(PWD)/output:/data/output:rw \
		$(DOCKER_IMAGE):$(DOCKER_TAG)

.PHONY: docker-dev
docker-dev: ## Start interactive development container
	@echo -e "$(BOLD)Starting development container...$(RESET)"
	@docker compose -f docker/docker-compose.yml run --rm truss-dev

.PHONY: docker-shell
docker-shell: ## Open shell in running container
	@echo -e "$(BOLD)Opening shell in container...$(RESET)"
	@docker run --rm -it \
		-v $(PWD):/workspace:rw \
		$(DOCKER_IMAGE):dev /bin/bash

.PHONY: docker-test
docker-test: ## Test Docker container
	@echo -e "$(BOLD)Testing Docker container...$(RESET)"
	@docker build -t $(DOCKER_IMAGE):test -f docker/Dockerfile .
	@docker run --rm $(DOCKER_IMAGE):test --help
	@echo -e "$(GREEN)✓ Container test passed$(RESET)"

.PHONY: docker-push
docker-push: ## Push Docker image to registry
	@echo -e "$(BOLD)Pushing Docker image to registry...$(RESET)"
	@docker tag $(DOCKER_IMAGE):$(DOCKER_TAG) $(REGISTRY)/$(DOCKER_IMAGE):$(DOCKER_TAG)
	@docker push $(REGISTRY)/$(DOCKER_IMAGE):$(DOCKER_TAG)
	@echo -e "$(GREEN)✓ Image pushed: $(REGISTRY)/$(DOCKER_IMAGE):$(DOCKER_TAG)$(RESET)"

.PHONY: docker-clean
docker-clean: ## Remove project Docker images, containers, and volumes
	@echo -e "$(BOLD)Cleaning project Docker artifacts...$(RESET)"
	@echo "Stopping and removing project containers..."
	@docker compose -f docker/docker-compose.yml down -v 2>/dev/null || true
	@docker stop truss-analysis truss-dev 2>/dev/null || true
	@docker rm truss-analysis truss-dev 2>/dev/null || true
	@echo "Removing project images..."
	@docker rmi $(DOCKER_IMAGE):$(DOCKER_TAG) 2>/dev/null || true
	@docker rmi $(DOCKER_IMAGE):dev 2>/dev/null || true
	@docker rmi $(DOCKER_IMAGE):test 2>/dev/null || true
	@echo "Removing project build cache volume..."
	@docker volume rm 2d-truss-analysis-cpp_build-cache 2>/dev/null || true
	@echo -e "$(GREEN)✓ Project Docker artifacts cleaned$(RESET)"

.PHONY: docker-size
docker-size: ## Show Docker image sizes
	@echo -e "$(BOLD)Docker Image Sizes:$(RESET)"
	@docker images | grep $(DOCKER_IMAGE) || echo "No images found"

# ==============================================================================
# CI/CD Targets
# ==============================================================================

.PHONY: ci
ci: build test format-check ## CI pipeline (build + test + format check)
	@echo -e "$(GREEN)✓ CI pipeline passed$(RESET)"

.PHONY: ci-full
ci-full: ci coverage lint static-analysis ## Full CI pipeline (build + test + coverage + analysis)
	@echo -e "$(GREEN)✓ Full CI pipeline passed$(RESET)"

# ==============================================================================
# Phony Target Declaration
# ==============================================================================

.PHONY: help all build debug rebuild rebuild-debug test test-unit test-integration \
        test-gui test-verbose test-debug coverage coverage-open format format-check \
        lint static-analysis docs run-cli run-gui install install-ubuntu install-fedora \
        info clean clean-debug clean-coverage clean-all distclean \
        docker-build docker-build-dev docker-run docker-dev docker-shell \
        docker-test docker-push docker-clean docker-size ci ci-full
