/**
 * @file test_main.cpp
 * @brief Custom GTest main() for the unit_tests binary.
 *
 * ## Why this file exists
 *
 * The unit_tests binary contains Qt-based tests (models, controllers) that
 * each declare a file-local `ensureQApp()` helper.  The helpers allocate a
 * QCoreApplication with `new` and deliberately never delete it, following
 * the Qt pattern of "one application for the process lifetime".  However,
 * this leaves `QCoreApplication::~QCoreApplication()` uncalled at process
 * exit — Qt's internal cleanup routines (registered via qAddPostRoutine and
 * Q_GLOBAL_STATIC) then execute during the C++ static-destructor / atexit
 * phase in an unspecified order relative to the gcov profiling library's own
 * atexit handler that flushes .gcda coverage data files.
 *
 * When the contest happens, the gcov runtime may encounter partially-written
 * or logically-inconsistent coverage data and call abort(), which CTest
 * reports as "Exception: SegFault".  The failure is intermittent and order-
 * dependent — it is most reliably triggered after rebuilding the project
 * without first deleting stale .gcda files, because the new binary has a
 * different counter-structure that the "corrupt arc tag" check rejects.
 *
 * ## The fix
 *
 * Constructing QCoreApplication **on the stack** in main() means its
 * destructor runs when main() returns — deterministically before any
 * atexit handler.  Qt finishes all of its cleanup first; gcov then gets an
 * exclusive, uncontested flush of the .gcda files.
 *
 * Each per-file ensureQApp() checks QCoreApplication::instance() before
 * allocating; since main() creates the instance up front, the helpers simply
 * return a reference to it and never call `new`.
 *
 * @note This file is only compiled into unit_tests when BUILD_GUI=ON (the
 *       only configuration that links Qt and therefore needs a QCoreApp).
 *       When BUILD_GUI=OFF the binary has no Qt dependency and the standard
 *       GTest::gtest_main is used instead.
 */

#include <QCoreApplication>

#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
    // Stack-allocated QCoreApplication: destroyed when main() returns,
    // before the atexit / static-destructor competition begins.
    //
    // QT_QPA_PLATFORM=offscreen is injected by the CMake ENVIRONMENT test
    // property, so the application finds the offscreen platform plugin
    // automatically without any manual setenv() call here.
    QCoreApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
