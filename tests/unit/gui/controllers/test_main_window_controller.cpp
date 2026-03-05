/**
 * @file test_main_window_controller.cpp
 * @brief Unit tests for MainWindowController (Phase 4).
 *
 * Verifies:
 *  - Constructor initialises to Empty phase.
 *  - setState() emits stateChanged exactly once on change, zero times on no-op.
 *  - state() accessor returns the current state.
 *  - onTrussModified() transitions to ModelBuilding and stores the handle.
 *  - onAnalysisCompleted() transitions to ResultsReady and stores the handle.
 *  - onAnalysisFailed() transitions back to ModelBuilding and records the error.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-02
 */

#include "gui/controllers/main_window_controller.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include "application/result.hpp"
#include "core/interfaces/ianalysis_results_view.hpp"
#include "core/interfaces/itruss_view.hpp"
#include "core/validation/truss_validator.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::gui::ctrl;
using namespace truss::gui::state;
using truss::test::MockTrussAnalysisFacade;

// ============================================================
// QApplication bootstrap (shared across all GUI test files)
// ============================================================

namespace {
QApplication& ensureQApp()
{
    static int   s_argc    = 1;
    static char  s_argv0[] = "unit_tests";
    static char* s_argv[]  = {s_argv0, nullptr};
    static QApplication* s_app = []() -> QApplication* {
        if (auto* existing = qobject_cast<QApplication*>(QCoreApplication::instance()))
            return existing;
        return new QApplication(s_argc, s_argv);
    }();
    return *s_app;
}

// ============================================================
// Minimal ITrussView stub — 0 nodes, 0 members
// ============================================================

class StubTrussView final : public truss::core::interfaces::ITrussView {
public:
    const std::string& getName() const override { return m_name; }
    std::vector<truss::core::interfaces::NodeView>
        getNodeViews()   const override { return {}; }
    std::size_t getNodeCount()   const override { return 0; }
    std::vector<truss::core::interfaces::MemberView>
        getMemberViews() const override { return {}; }
    std::size_t getMemberCount() const override { return 0; }
    std::size_t getTotalDofs()   const override { return 0; }
    std::size_t getFreeDofs()    const override { return 0; }
    std::size_t getConstrainedDofs() const override { return 0; }
private:
    std::string m_name{"StubTruss"};
};

}  // namespace

// ============================================================
// Fixture
// ============================================================

class MainWindowControllerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ensureQApp();
        // MockTrussAnalysisFacade uses NiceMock semantics (no unexpected-call warnings
        // needed for this set of tests — we only care about controller behaviour)
        controller = std::make_unique<MainWindowController>(facade);
    }

    void TearDown() override
    {
        controller.reset();
    }

    ::testing::NiceMock<MockTrussAnalysisFacade> facade;
    std::unique_ptr<MainWindowController>        controller;
};

// ============================================================
// Tests — initial state
// ============================================================

TEST_F(MainWindowControllerTest, InitialPhaseIsEmpty)
{
    EXPECT_EQ(controller->state().phase, WorkspacePhase::Empty);
}

TEST_F(MainWindowControllerTest, InitialHandlesAreZero)
{
    EXPECT_EQ(controller->state().trussHandle,   0u);
    EXPECT_EQ(controller->state().resultsHandle, 0u);
}

TEST_F(MainWindowControllerTest, InitialIsDirtyIsFalse)
{
    EXPECT_FALSE(controller->state().isDirty);
}

TEST_F(MainWindowControllerTest, InitialLastErrorIsEmpty)
{
    EXPECT_TRUE(controller->state().lastError.empty());
}

// ============================================================
// Tests — setState / stateChanged signal
// ============================================================

TEST_F(MainWindowControllerTest, SetStateEmitsStateChangedSignal)
{
    QSignalSpy spy(controller.get(), &MainWindowController::stateChanged);

    WorkspaceState s;
    s.phase = WorkspacePhase::ModelBuilding;
    controller->setState(s);

    ASSERT_EQ(spy.count(), 1);
    const auto emitted = qvariant_cast<WorkspaceState>(spy.at(0).at(0));
    EXPECT_EQ(emitted.phase, WorkspacePhase::ModelBuilding);
}

TEST_F(MainWindowControllerTest, SetStateSameValueDoesNotEmitSignal)
{
    QSignalSpy spy(controller.get(), &MainWindowController::stateChanged);

    // Set to default Empty state again — no change
    controller->setState(WorkspaceState{});

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(MainWindowControllerTest, StateAccessorReturnsLatestState)
{
    WorkspaceState s;
    s.phase        = WorkspacePhase::ResultsReady;
    s.trussHandle  = 42u;
    s.resultsHandle = 7u;
    controller->setState(s);

    EXPECT_EQ(controller->state().phase,         WorkspacePhase::ResultsReady);
    EXPECT_EQ(controller->state().trussHandle,    42u);
    EXPECT_EQ(controller->state().resultsHandle,  7u);
}

TEST_F(MainWindowControllerTest, SetStateMultipleTransitionsEmitsForEachChange)
{
    QSignalSpy spy(controller.get(), &MainWindowController::stateChanged);

    WorkspaceState s1;
    s1.phase = WorkspacePhase::ModelBuilding;
    controller->setState(s1);

    WorkspaceState s2;
    s2.phase = WorkspacePhase::Analysing;
    controller->setState(s2);

    EXPECT_EQ(spy.count(), 2);
}

// ============================================================
// Tests — onTrussModified slot
// ============================================================

TEST_F(MainWindowControllerTest, OnTrussModifiedTransitionsToModelBuilding)
{
    controller->onTrussModified(99u);

    EXPECT_EQ(controller->state().phase,        WorkspacePhase::ModelBuilding);
    EXPECT_EQ(controller->state().trussHandle,  99u);
}

TEST_F(MainWindowControllerTest, OnTrussModifiedSetsIsDirty)
{
    controller->onTrussModified(1u);
    EXPECT_TRUE(controller->state().isDirty);
}

TEST_F(MainWindowControllerTest, OnTrussModifiedClearsResultsHandle)
{
    // Simulate prior ResultsReady state
    WorkspaceState s;
    s.phase         = WorkspacePhase::ResultsReady;
    s.resultsHandle = 55u;
    s.trussHandle   = 1u;
    controller->setState(s);

    controller->onTrussModified(1u);

    EXPECT_EQ(controller->state().resultsHandle, 0u);
}

TEST_F(MainWindowControllerTest, OnTrussModifiedEmitsStateChanged)
{
    QSignalSpy spy(controller.get(), &MainWindowController::stateChanged);
    controller->onTrussModified(5u);
    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// Tests — onAnalysisCompleted slot
// ============================================================

TEST_F(MainWindowControllerTest, OnAnalysisCompletedTransitionsToResultsReady)
{
    // Start from ModelBuilding (so the transition is not a no-op)
    WorkspaceState s;
    s.phase       = WorkspacePhase::Analysing;
    s.trussHandle = 10u;
    controller->setState(s);

    controller->onAnalysisCompleted(77u);

    EXPECT_EQ(controller->state().phase,         WorkspacePhase::ResultsReady);
    EXPECT_EQ(controller->state().resultsHandle, 77u);
}

TEST_F(MainWindowControllerTest, OnAnalysisCompletedClearsLastError)
{
    WorkspaceState s;
    s.lastError = "prior error";
    s.phase     = WorkspacePhase::Analysing;
    controller->setState(s);

    controller->onAnalysisCompleted(1u);

    EXPECT_TRUE(controller->state().lastError.empty());
}

// ============================================================
// Tests — onAnalysisFailed slot
// ============================================================

TEST_F(MainWindowControllerTest, OnAnalysisFailedTransitionsToModelBuilding)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::Analysing;
    controller->setState(s);

    controller->onAnalysisFailed(QStringLiteral("Singular stiffness matrix"));

    EXPECT_EQ(controller->state().phase, WorkspacePhase::ModelBuilding);
}

TEST_F(MainWindowControllerTest, OnAnalysisFailedStoresErrorMessage)
{
    WorkspaceState s;
    s.phase = WorkspacePhase::Analysing;
    controller->setState(s);

    controller->onAnalysisFailed(QStringLiteral("Divide by zero"));

    EXPECT_EQ(controller->state().lastError, "Divide by zero");
}

TEST_F(MainWindowControllerTest, OnAnalysisFailedClearsResultsHandle)
{
    WorkspaceState s;
    s.phase         = WorkspacePhase::Analysing;
    s.resultsHandle = 3u;
    controller->setState(s);

    controller->onAnalysisFailed(QStringLiteral("error"));

    EXPECT_EQ(controller->state().resultsHandle, 0u);
}
