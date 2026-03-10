# CMake Build System Modularization and Developer Toolchain Standardization

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 11, 2026  
**Status:** ✅ Complete  
**Branch:** `refactor/standardize-build-system-and-toolchain`  
**Scope:** CMake modularization, Makefile hardening, build script standardization

---

## 1. Executive Summary

This work log documents the decomposition of the project's monolithic root `CMakeLists.txt` into a fully modular, per-directory CMake build system, and the simultaneous elimination of CMake anti-patterns from the top-level `Makefile` and all helper scripts in `scripts/`.

The root `CMakeLists.txt` had grown to over 1,000 lines, directly encoding all target definitions, source file lists, compiler flags, and test configuration in a single file. This violated the CMake principle of keeping build logic co-located with source code and made the file difficult to maintain as the codebase's layered architecture expanded.

In parallel, the `Makefile` and several helper scripts contained redundant reimplementations of functionality that CMake already manages: generator detection, CPU core probing, direct `ninja`/`make` invocation, and manual job count propagation. These were eliminated so that the Makefile and scripts serve exclusively as thin orchestration wrappers above CMake.

---

## 2. Scope of Work

### CMake Modularization

- Decomposed the monolithic root `CMakeLists.txt` into 12 per-directory `CMakeLists.txt` files
- Defined one library target per architectural layer, each in its own subdirectory
- Created `src/CMakeLists.txt` and `tests/CMakeLists.txt` as top-level orchestrators
- Reduced the root `CMakeLists.txt` from ~1,018 lines to ~370 lines (project bootstrap and orchestration only)

### Makefile Hardening

- Removed `NPROC` CPU core detection
- Removed `CMAKE_GENERATOR` runtime detection and `ifeq`/`else` `BUILD_TOOL` selection
- Removed `CXX` compiler detection
- Removed the `--include-dir=.` `MAKEFLAGS` hack and duplicate `.SUFFIXES:` declaration
- Replaced all direct tool invocations with `cmake --build --parallel` and `ctest --test-dir --parallel`
- Introduced `CMAKE ?= cmake` and `CTEST ?= ctest` override variables for CI environments
- Simplified `install` target to `cmake --install <dir>` (eliminated `cd` subshell)
- Simplified `info` target to report CMake version rather than duplicating generator/compiler state

### Build Script Standardization

- Replaced `cd "$BUILD_DIR" && cmake .. -D...` with `cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -D...` throughout
- Replaced `cmake --build . -j"$PARALLEL_JOBS"` with `cmake --build "$BUILD_DIR" --parallel`
- Removed `-j <N>` / `--jobs` CLI option and `NPROC` detection from `build.sh`
- Replaced `make clean` and `make -j$NPROC` with `cmake --build --target clean` and `cmake --build --parallel` in `generate_coverage.sh`
- Replaced bare `ctest --output-on-failure` (invoked from `cd $BUILD_DIR`) with `ctest --test-dir "$BUILD_DIR" --output-on-failure --parallel` in `test.sh`
- Replaced `cd "$BUILD_DIR" && cmake --build .` with `cmake --build "$BUILD_DIR"` in `generate-docs.sh`
- Converted all hardcoded relative paths in `generate_coverage.sh` to absolute paths derived from `$SCRIPT_DIR` and `$PROJECT_DIR`
- Hardened all six scripts from `set -e` to `set -euo pipefail`

### Defect Corrections

- Fixed `FileLogger` member initializer list ordering to match declaration order (suppressed compiler warning)
- Corrected test binary paths in `generate_coverage.sh` following the modular refactor relocating binaries from `$BUILD_DIR/` to `$BUILD_DIR/tests/unit/` and `$BUILD_DIR/tests/integration/`

---

## 3. Technical Changes

### Modular CMake Target Hierarchy

The following library targets were extracted, each defined in its own `CMakeLists.txt`:

| Target                | Directory             | Role                                        |
| --------------------- | --------------------- | ------------------------------------------- |
| `TrussUtils`          | `src/utilities/`      | String and math utilities                   |
| `TrussCore`           | `src/core/`           | Domain model (pure, no I/O)                 |
| `TrussInfrastructure` | `src/infrastructure/` | I/O, logging, export adapters               |
| `TrussApplication`    | `src/application/`    | Application services                        |
| `TrussInterface`      | `src/interface/`      | Public facade and builder API               |
| `TrussCLI`            | `src/cli/`            | CLI library + `TrussAnalysisCLI` executable |
| `TrussGui`            | `src/gui/`            | GUI library + `TrussAnalysisGUI` executable |

Dependency direction is strictly one-way:

```
TrussUtils → TrussCore → TrussInfrastructure → TrussApplication → TrussInterface → TrussCLI
                                                                                  → TrussGui
```

`AUTOMOC`/`AUTORCC`/`AUTOUIC` are scoped to `TrussGui` and `TrussAnalysisGUI` only, not set globally.

### Test Target Configuration

- `tests/CMakeLists.txt`: smoke test (`test_gtest_integration`) and `all_gtest_tests` meta-target
- `tests/unit/CMakeLists.txt`: `unit_tests` + optional `unit_tests_gui_widgets` (guarded by `BUILD_GUI`)
- `tests/integration/CMakeLists.txt`: `integration_tests` + optional `gui_integration_test` (guarded by `BUILD_GUI`)

Each test target links only its top-level transitive dependency (`TrussCLI` or `TrussGui`) plus `GTest::gtest_main` and `GTest::gmock`, eliminating duplicate transitive library links that generated linker warnings on macOS.

### Root CMakeLists.txt Structure (Post-Refactor)

The root file retains only:

- CMake version and project declaration
- Compiler standard and warning flags
- `find_package` calls for all third-party dependencies
- GTest/GMock target fixup for Ubuntu 24.04
- `add_subdirectory(src)` and `add_subdirectory(tests)` (BUILD_TESTING-guarded)
- Install rules, CPack configuration, Doxygen target, and coverage custom targets

### Correct Build Orchestration Pattern (Post-Refactor)

```
make <target>
    ↓
scripts/build.sh  /  scripts/test.sh  /  …
    ↓
cmake -S . -B build  /  cmake --build build --parallel  /  ctest --test-dir build --parallel
    ↓
CMake (selects generator from cache: Ninja / Unix Makefiles / etc.)
    ↓
Ninja (or configured generator)
```

---

## 4. Testing and Validation

- Full build: 119 compilation units, 0 errors, 0 warnings after refactor.
- `ctest`: 3/3 tests pass (`test_gtest_integration`, `unit_tests`, `integration_tests`).
- Zero duplicate-library linker warnings (verified by grepping build output for `duplicate`).
- All six refactored scripts pass `bash -n` syntax validation.
- Confirmed no remaining `nproc`, `NPROC`, `PARALLEL_JOBS`, `make clean`, `make -j`, `ninja`, or `cd "$BUILD_DIR"` + `cmake ..` patterns across `Makefile` and all scripts.
- All existing `make` targets (`build`, `debug`, `test`, `coverage`, `format`, `docs`, `ci`, `install`, etc.) remain functional with no interface changes.

---

## 5. Outcome

The build system is now fully modular and generator-agnostic. Each source and test directory owns its own `CMakeLists.txt`, build logic is co-located with the code it describes, and the root file is reduced to a clean project orchestrator. The `Makefile` and all helper scripts are thin workflow wrappers that delegate all build orchestration to CMake without reimplementing any of its responsibilities. No public API or library interfaces were modified. No breaking changes were introduced.
