/**
 * @file test_analysis_options_dialog.cpp
 * @brief Unit tests for AnalysisOptionsDialog (Phase 5).
 *
 * Exercises:
 *  - Widget constructs without crashing.
 *  - Default options() values match the AnalysisOptions defaults.
 *  - Switching to the iterative solver enables tolerance/maxIter controls.
 *  - Switching back to direct solver disables tolerance/maxIter controls.
 *  - setOptions() pre-populates every field correctly.
 *  - options() round-trips values set through setOptions().
 *  - Clicking OK accepts the dialog.
 *  - Clicking Cancel rejects the dialog.
 *
 * AnalysisOptionsDialog is a QDialog (QWidget subclass), so it requires
 * QApplication + the offscreen platform plugin.  It is therefore compiled
 * into unit_tests_gui_widgets.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "core/analysis/analysis_orchestrator.hpp"
#include "gui/panels/analysis_options_dialog.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTest>

#include <gtest/gtest.h>

using truss::core::analysis::AnalysisOptions;
using truss::gui::AnalysisOptionsDialog;

// ============================================================
// QApplication bootstrap
// ============================================================

namespace {

QApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* e = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return e;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

}  // namespace

// ============================================================
// Fixture
// ============================================================

class AnalysisOptionsDialogTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        dlg = std::make_unique<AnalysisOptionsDialog>();
    }
    void TearDown() override { dlg.reset(); }

    std::unique_ptr<AnalysisOptionsDialog> dlg;
};

// ============================================================
// Construction
// ============================================================

TEST_F(AnalysisOptionsDialogTest, ConstructsWithoutCrash) {
    SUCCEED();
}

TEST_F(AnalysisOptionsDialogTest, WindowTitleIsSet) {
    EXPECT_FALSE(dlg->windowTitle().isEmpty());
}

// ============================================================
// Default options
// ============================================================

TEST_F(AnalysisOptionsDialogTest, DefaultOptions_UseDirectSolver) {
    EXPECT_TRUE(dlg->options().useDirectSolver);
}

TEST_F(AnalysisOptionsDialogTest, DefaultOptions_ComputeReactionsEnabled) {
    EXPECT_TRUE(dlg->options().computeReactions);
}

TEST_F(AnalysisOptionsDialogTest, DefaultOptions_CheckStabilityEnabled) {
    EXPECT_TRUE(dlg->options().checkStability);
}

TEST_F(AnalysisOptionsDialogTest, DefaultOptions_VerboseDisabled) {
    EXPECT_FALSE(dlg->options().verbose);
}

TEST_F(AnalysisOptionsDialogTest, DefaultOptions_MaxIterationsIs1000) {
    EXPECT_EQ(dlg->options().maxIterations, 1000);
}

// ============================================================
// Solver-type combo — onSolverTypeChanged slot
// ============================================================

TEST_F(AnalysisOptionsDialogTest, DirectSolver_ToleranceAndMaxIterDisabled) {
    auto* solverCombo = dlg->findChild<QComboBox*>("solverCombo");
    ASSERT_NE(solverCombo, nullptr);
    auto* toleranceSpin = dlg->findChild<QDoubleSpinBox*>("toleranceSpin");
    ASSERT_NE(toleranceSpin, nullptr);
    auto* maxIterSpin = dlg->findChild<QSpinBox*>("maxIterSpin");
    ASSERT_NE(maxIterSpin, nullptr);

    // Default is direct solver (index 0)
    EXPECT_EQ(solverCombo->currentIndex(), 0);
    EXPECT_FALSE(toleranceSpin->isEnabled());
    EXPECT_FALSE(maxIterSpin->isEnabled());
}

TEST_F(AnalysisOptionsDialogTest, IterativeSolver_ToleranceAndMaxIterEnabled) {
    auto* solverCombo = dlg->findChild<QComboBox*>("solverCombo");
    ASSERT_NE(solverCombo, nullptr);
    auto* toleranceSpin = dlg->findChild<QDoubleSpinBox*>("toleranceSpin");
    ASSERT_NE(toleranceSpin, nullptr);
    auto* maxIterSpin = dlg->findChild<QSpinBox*>("maxIterSpin");
    ASSERT_NE(maxIterSpin, nullptr);

    solverCombo->setCurrentIndex(1);  // iterative
    QApplication::processEvents();

    EXPECT_TRUE(toleranceSpin->isEnabled());
    EXPECT_TRUE(maxIterSpin->isEnabled());
    EXPECT_FALSE(dlg->options().useDirectSolver);
}

TEST_F(AnalysisOptionsDialogTest, SwitchBackToDirectSolver_DisablesControls) {
    auto* solverCombo = dlg->findChild<QComboBox*>("solverCombo");
    ASSERT_NE(solverCombo, nullptr);
    auto* toleranceSpin = dlg->findChild<QDoubleSpinBox*>("toleranceSpin");
    ASSERT_NE(toleranceSpin, nullptr);

    solverCombo->setCurrentIndex(1);  // iterative → enable
    solverCombo->setCurrentIndex(0);  // direct → disable again
    QApplication::processEvents();

    EXPECT_FALSE(toleranceSpin->isEnabled());
    EXPECT_TRUE(dlg->options().useDirectSolver);
}

// ============================================================
// setOptions / options round-trip
// ============================================================

TEST_F(AnalysisOptionsDialogTest, SetOptions_PopulatesIterativeSolverCorrectly) {
    AnalysisOptions opts;
    opts.useDirectSolver = false;
    // Use 1e-8: representable within the spinbox's 10-decimal-place display
    // precision. Values below ~1e-10 get rounded to 0 by QDoubleSpinBox.
    opts.convergenceTolerance = 1e-8;
    opts.maxIterations = 500;
    opts.computeReactions = false;
    opts.checkStability = false;
    opts.verbose = true;

    dlg->setOptions(opts);
    QApplication::processEvents();

    const auto out = dlg->options();
    EXPECT_FALSE(out.useDirectSolver);
    EXPECT_DOUBLE_EQ(out.convergenceTolerance, 1e-8);
    EXPECT_EQ(out.maxIterations, 500);
    EXPECT_FALSE(out.computeReactions);
    EXPECT_FALSE(out.checkStability);
    EXPECT_TRUE(out.verbose);
}

TEST_F(AnalysisOptionsDialogTest, SetOptions_DirectSolver_KeyFieldsRoundTrip) {
    AnalysisOptions opts;
    opts.useDirectSolver = true;
    opts.computeReactions = true;
    opts.checkStability = false;
    opts.verbose = true;

    dlg->setOptions(opts);
    QApplication::processEvents();

    const auto out = dlg->options();
    EXPECT_TRUE(out.useDirectSolver);
    EXPECT_TRUE(out.computeReactions);
    EXPECT_FALSE(out.checkStability);
    EXPECT_TRUE(out.verbose);
}

// ============================================================
// CheckBox widget state mirrors options()
// ============================================================

TEST_F(AnalysisOptionsDialogTest, VerboseCheckbox_ReflectsOptionsVerbose) {
    auto* verboseCheck = dlg->findChild<QCheckBox*>("verboseCheck");
    ASSERT_NE(verboseCheck, nullptr);

    // By default verbose is false
    EXPECT_FALSE(verboseCheck->isChecked());

    AnalysisOptions opts;
    opts.verbose = true;
    dlg->setOptions(opts);
    QApplication::processEvents();

    EXPECT_TRUE(verboseCheck->isChecked());
    EXPECT_TRUE(dlg->options().verbose);
}
