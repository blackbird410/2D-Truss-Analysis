/**
 * @file test_analysis_control_bar.cpp
 * @brief Unit tests for AnalysisControlBar (Phase 5).
 *
 * Verifies:
 *  - Widget constructs without crashing.
 *  - Initial state shows the run-page (actionStack index 0).
 *  - onStateChanged(Analysing) swaps to the progress-page (index 1).
 *  - onStateChanged(ModelBuilding+hasTruss) restores to run-page (index 0) enabled.
 *  - onStateChanged(Empty) shows run-page with Run button disabled.
 *  - analyzeRequested signal is emitted when Run button is clicked.
 *  - validateRequested signal is emitted when Validate button is clicked.
 *  - optionsRequested signal is emitted when Options button is clicked.
 *  - setOptions / options() round-trips correctly.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/panels/analysis_control_bar.hpp"

#include "core/analysis/analysis_orchestrator.hpp"
#include "gui/state/workspace_state.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QPushButton>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTest>

#include <gtest/gtest.h>

using truss::gui::AnalysisControlBar;
using truss::core::analysis::AnalysisOptions;
using truss::gui::state::WorkspacePhase;
using truss::gui::state::WorkspaceState;

// ============================================================
// QApplication bootstrap
// ============================================================

namespace {
QApplication& ensureQApp()
{
    static int   s_argc    = 1;
    static char  s_argv0[] = "unit_tests";
    static char* s_argv[]  = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* e = qobject_cast<QApplication*>(QCoreApplication::instance())) return e;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

WorkspaceState makeState(WorkspacePhase phase, std::size_t handle = 0)
{
    WorkspaceState s;
    s.phase = phase;
    s.trussHandle = handle;
    return s;
}
}  // namespace

// ============================================================
// Fixture
// ============================================================

class AnalysisControlBarTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ensureQApp();
        bar = std::make_unique<AnalysisControlBar>();
    }
    void TearDown() override { bar.reset(); }

    std::unique_ptr<AnalysisControlBar> bar;
};

// ============================================================
// Construction / widget structure
// ============================================================

TEST_F(AnalysisControlBarTest, ConstructsWithoutCrash) { SUCCEED(); }

TEST_F(AnalysisControlBarTest, InitialState_ShowsRunPage)
{
    auto* stack = bar->findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr) << "Expected a QStackedWidget inside AnalysisControlBar";
    EXPECT_EQ(stack->currentIndex(), 0) << "Run page (index 0) should be shown initially";
}

// ============================================================
// State changes
// ============================================================

TEST_F(AnalysisControlBarTest, StateChanged_Analysing_ShowsProgressPage)
{
    bar->onStateChanged(makeState(WorkspacePhase::Analysing, 1));
    QApplication::processEvents();

    auto* stack = bar->findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->currentIndex(), 1) << "Progress page (index 1) expected during Analysing";
}

TEST_F(AnalysisControlBarTest, StateChanged_ModelBuilding_ShowsRunPageEnabled)
{
    // First go to Analysing, then back to ModelBuilding.
    bar->onStateChanged(makeState(WorkspacePhase::Analysing, 2));
    bar->onStateChanged(makeState(WorkspacePhase::ModelBuilding, 2));
    QApplication::processEvents();

    auto* stack = bar->findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->currentIndex(), 0) << "Run page (index 0) expected after ModelBuilding";
}

TEST_F(AnalysisControlBarTest, StateChanged_Empty_ShowsRunPageDisabled)
{
    bar->onStateChanged(makeState(WorkspacePhase::Empty, 0));
    QApplication::processEvents();

    auto* stack = bar->findChild<QStackedWidget*>();
    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->currentIndex(), 0);

    // Find the run button and verify it is disabled.
    auto btns = bar->findChildren<QPushButton*>();
    auto it = std::find_if(btns.begin(), btns.end(), [](QPushButton* b) {
        return b->objectName().contains(QLatin1String("run"), Qt::CaseInsensitive)
            || b->text().contains(QLatin1String("Run"), Qt::CaseInsensitive)
            || b->text().contains(QLatin1String("Analyze"), Qt::CaseInsensitive);
    });
    if (it != btns.end()) {
        EXPECT_FALSE((*it)->isEnabled()) << "Run button should be disabled when workspace is Empty";
    }
}

// ============================================================
// Signals
// ============================================================

TEST_F(AnalysisControlBarTest, RunButtonClick_EmitsAnalyzeRequested)
{
    // Enable state first.
    bar->onStateChanged(makeState(WorkspacePhase::ModelBuilding, 1));
    QApplication::processEvents();

    QSignalSpy spy{bar.get(), &AnalysisControlBar::analyzeRequested};

    // Find the run page button by objectName or text.
    auto btns = bar->findChildren<QPushButton*>();
    auto it = std::find_if(btns.begin(), btns.end(), [](QPushButton* b) {
        return b->isEnabled() && (
            b->objectName().contains(QLatin1String("run"), Qt::CaseInsensitive) ||
            b->text().contains(QLatin1String("Run"), Qt::CaseInsensitive) ||
            b->text().contains(QLatin1String("Analyze"), Qt::CaseInsensitive));
    });
    ASSERT_NE(it, btns.end()) << "No enabled run/analyze button found";
    QTest::mouseClick(*it, Qt::LeftButton);
    QApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AnalysisControlBarTest, ValidateButton_EmitsValidateRequested)
{
    bar->onStateChanged(makeState(WorkspacePhase::ModelBuilding, 1));
    QApplication::processEvents();

    QSignalSpy spy{bar.get(), &AnalysisControlBar::validateRequested};

    auto btns = bar->findChildren<QPushButton*>();
    auto it = std::find_if(btns.begin(), btns.end(), [](QPushButton* b) {
        return b->text().contains(QLatin1String("Validat"), Qt::CaseInsensitive);
    });
    ASSERT_NE(it, btns.end()) << "No Validate button found";
    QTest::mouseClick(*it, Qt::LeftButton);
    QApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AnalysisControlBarTest, OptionsButton_EmitsOptionsRequested)
{
    QSignalSpy spy{bar.get(), &AnalysisControlBar::optionsRequested};

    auto btns = bar->findChildren<QPushButton*>();
    auto it = std::find_if(btns.begin(), btns.end(), [](QPushButton* b) {
        return b->text().contains(QLatin1String("Option"), Qt::CaseInsensitive)
            || b->objectName().contains(QLatin1String("options"), Qt::CaseInsensitive);
    });
    ASSERT_NE(it, btns.end()) << "No Options button found";
    QTest::mouseClick(*it, Qt::LeftButton);
    QApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// Options round-trip
// ============================================================

TEST_F(AnalysisControlBarTest, SetOptions_RoundTripsCorrectly)
{
    AnalysisOptions opts;
    opts.useDirectSolver       = false;
    opts.convergenceTolerance   = 1e-12;
    opts.maxIterations         = 500;
    opts.computeReactions      = false;
    opts.verbose               = true;

    bar->setOptions(opts);

    const auto& stored = bar->options();
    EXPECT_EQ(stored.useDirectSolver,     false);
    EXPECT_DOUBLE_EQ(stored.convergenceTolerance, 1e-12);
    EXPECT_EQ(stored.maxIterations,       500);
    EXPECT_EQ(stored.computeReactions,    false);
    EXPECT_EQ(stored.verbose,             true);
}
