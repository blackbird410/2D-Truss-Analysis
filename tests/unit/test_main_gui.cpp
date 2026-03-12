/**
 * @file test_main_gui.cpp
 * @brief Custom GTest main() for the unit_tests_gui_widgets binary.
 *
 * Creates a stack-allocated QApplication so Qt's teardown sequence runs
 * deterministically when main() returns — before gcov's atexit handler
 * flushes .gcda coverage data files (same rationale as test_main.cpp).
 *
 * Each per-file ensureQApp() helper in this binary checks
 * QCoreApplication::instance() / qobject_cast<QApplication*>(...) before
 * allocating.  Because main() creates the QApplication first, all helpers
 * reuse it without calling `new`.
 *
 * QT_QPA_PLATFORM=offscreen is set by the CMake ENVIRONMENT test property;
 * this allows widget rendering without a physical display server.
 */

#include <QApplication>

#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
    // Stack-allocated QApplication: destroyed when main() returns,
    // ensuring Qt cleanup happens before any atexit/global-destructor phase.
    QApplication app(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
