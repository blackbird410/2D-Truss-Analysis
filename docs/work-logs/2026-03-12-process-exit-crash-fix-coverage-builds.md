# Process-Exit Crash Fix in Coverage-Instrumented Builds

**Project:** 2D Truss Analysis C++ v3.0.0  
**Date:** March 12, 2026  
**Status:** ✅ Complete  
**Branch:** `build/fix-process-exit-segfault`  
**Scope:** Coverage build stability, CTest fixture infrastructure, GTest entry point correctness

---

## 1. Executive Summary

This work log documents the diagnosis and resolution of a pre-existing `abort()` at process exit in coverage-instrumented builds. CTest reported the failure as "Exception: SegFault" for the `unit_tests` binary when the full test suite was executed after a rebuild that left stale `.gcda` files on disk. The crash did not reproduce when `unit_tests` was run in isolation or when the build tree was clean.

Root cause analysis identified two independent failure paths: a non-deterministic ordering race between Qt's atexit cleanup routines and the gcov profiling library's `.gcda` flush handler; and the gcov runtime's `gcov_error()` / `abort()` path triggered when a rebuilt binary's arc-counter structure does not match the checksum embedded in a pre-existing `.gcda` file ("corrupt arc tag"). Both failure paths were eliminated.

---

## 2. Scope of Work

### Testing

- Authored `tests/unit/test_main.cpp`: custom GTest `main()` for `unit_tests` with `QCoreApplication` constructed on the stack, ensuring deterministic Qt teardown before any atexit handler
- Authored `tests/unit/test_main_gui.cpp`: equivalent custom entry point for `unit_tests_gui_widgets` using stack-allocated `QApplication`
- Extended `lcov` source filters to exclude Homebrew-installed Qt framework headers (`/opt/homebrew/`) from coverage reports on macOS

### Build System

- Added `cmake/CleanGcda.cmake`: a CMake script-mode (`-P`) utility that enumerates and removes all `.gcda` files under a parameterised `${BUILD_DIR}`, suitable for both CTest fixture and manual invocation
- Registered `cleanup_gcda` as a CTest test with `FIXTURES_SETUP gcda_cleanup` in `tests/CMakeLists.txt`
- Propagated `FIXTURES_REQUIRED gcda_cleanup` to all five test targets across `tests/CMakeLists.txt`, `tests/unit/CMakeLists.txt`, and `tests/integration/CMakeLists.txt`
- Transitioned `unit_tests` and `unit_tests_gui_widgets` from `GTest::gtest_main` to `GTest::gtest`; the `BUILD_GUI=OFF` configuration retains `GTest::gtest_main` via a CMake generator expression, preserving the no-Qt build path

---

## 3. Technical Changes

### Root Cause 1 — Qt/gcov atexit Ordering Race

Each test source file in `unit_tests` and `unit_tests_gui_widgets` contained a file-local `ensureQApp()` helper that constructed a `QCoreApplication` or `QApplication` via `new` and never deallocated it. Qt's platform plugin, event loop, and signal/slot infrastructure register cleanup routines through `qAddPostRoutine` and `Q_GLOBAL_STATIC`. With no explicit destructor call, these routines executed during the C++ static-destructor / atexit phase at process exit, competing with gcov's own atexit handler that flushes `.gcda` arc counters. The interleaving of Qt's global-state teardown with gcov's memory accesses terminated the process with `abort()`.

**Fix:** `tests/unit/test_main.cpp` constructs `QCoreApplication app(argc, argv)` on the stack. When `main()` returns after `RUN_ALL_TESTS()`, `app` is destroyed deterministically — prior to any atexit handler. All existing `ensureQApp()` helpers detect the live `QCoreApplication::instance()` and return immediately without allocating a new object. The same pattern is applied in `tests/unit/test_main_gui.cpp` for `QApplication`. Both binaries were migrated from `GTest::gtest_main` to `GTest::gtest`; the custom main sources are added to each target's source list via `target_sources()`.

### Root Cause 2 — Stale `.gcda` File Corruption

When a binary is rebuilt, gcov instrumentation assigns a new checksum to its arc-counter arrays. If `.gcda` files from the previous build remain on disk, the gcov runtime detects the mismatch on first write and calls `gcov_error()`, which unconditionally calls `abort()`. This failure was most reproducible on the first full suite run following an incremental rebuild.

**Fix:** `cmake/CleanGcda.cmake` uses `file(GLOB_RECURSE ...)` to enumerate and remove all `*.gcda` files under `${BUILD_DIR}`. The script is registered as a CTest test via `add_test()` and assigned `FIXTURES_SETUP gcda_cleanup`. Every other test target in the suite declares `FIXTURES_REQUIRED gcda_cleanup`, so CTest executes the cleanup unconditionally before any binary runs, including under parallel test scheduling.

### CMake 4.x Directory-Scope Constraint

CMake 4.x restricts `set_tests_properties()` to tests defined within the same `CMakeLists.txt` directory. `FIXTURES_REQUIRED` for `unit_tests` and `unit_tests_gui_widgets` is set in `tests/unit/CMakeLists.txt`; `FIXTURES_REQUIRED` for `integration_tests` and `gui_integration_tests` is set in `tests/integration/CMakeLists.txt`; `FIXTURES_REQUIRED` for `gtest_integration` and the `FIXTURES_SETUP` declaration for `cleanup_gcda` are both in `tests/CMakeLists.txt`.

---

## 4. Testing and Validation

- Full `ctest` run: 6/6 tests pass — `cleanup_gcda`, `gtest_integration`, `unit_tests`, `unit_tests_gui_widgets`, `integration_tests`, `gui_integration_tests`
- Regression scenario verified: source file modified, binary rebuilt with stale `.gcda` files present, full `ctest` run executed — 6/6 pass, no abort
- All 1,533 test assertions continue to pass; no test logic was modified
- Configure-time validation confirmed: no `set_tests_properties` errors under CMake 4.2.3
- `BUILD_GUI=OFF` configuration unaffected: retains `GTest::gtest_main` and no Qt linkage

---

## 5. Outcome

The test suite no longer crashes at process exit in coverage-instrumented builds. Qt application lifecycle management is now RAII-correct in both test binaries: a single stack-allocated application object provides the required global Qt state for the duration of the test run and is destroyed deterministically before any external runtime hooks execute. The CTest fixture provides a persistent, build-tool-independent safeguard against stale coverage data, eliminating the entire crash class for all future incremental rebuilds. No public API, library interfaces, or cross-target dependency edges were modified. No breaking changes were introduced.
