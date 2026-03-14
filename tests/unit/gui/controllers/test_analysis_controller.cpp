/**
 * @file test_analysis_controller.cpp
 * @brief Unit tests for AnalysisController (Phase 5 — background QThread).
 *
 * Verifies:
 *  - onAnalyzeRequested emits analysisStarted immediately.
 *  - Successful analysis delivers analysisCompleted with the results handle.
 *  - Failed analysis delivers analysisFailed with an error message.
 *  - No truss handle emits analysisFailed synchronously without starting a thread.
 *  - currentResultsHandle() returns the last successful handle after completion.
 *
 * @note Tests that involve the worker thread use QSignalSpy::wait(timeout)
 *       to block until the queued result is delivered to the main thread.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "core/analysis/analysis_orchestrator.hpp"
#include "core/model/truss.hpp"
#include "gui/controllers/analysis_controller.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <QCoreApplication>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::gui::ctrl;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using truss::application::Result;
using truss::core::Truss;
using truss::core::analysis::AnalysisOptions;
using truss::test::MockTrussAnalysisFacade;

// ============================================================
// QCoreApplication bootstrap
// AnalysisController is a QObject subclass that manages a background
// QThread. No widgets or display interaction required. QCoreApplication
// is sufficient; no Qt platform plugin is loaded in CI.
// ============================================================

namespace {
QCoreApplication& ensureQApp() {
    static int s_argc = 1;
    static char s_argv0[] = "unit_tests";
    static char* s_argv[] = {s_argv0, nullptr};
    static QCoreApplication* s_app = []() -> QCoreApplication* {
        if (QCoreApplication::instance())
            return QCoreApplication::instance();
        return new QCoreApplication(s_argc, s_argv);
    }();
    return *s_app;
}
}  // namespace

class AnalysisControllerV2Test : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        ctrl = std::make_unique<AnalysisController>(facade);
        ctrl->onTrussHandleUpdated(kHandle);
    }
    void TearDown() override { ctrl.reset(); }

    static constexpr std::size_t kHandle = 3;

    NiceMock<MockTrussAnalysisFacade> facade;
    Truss localTruss{"TestTruss"};
    std::unique_ptr<AnalysisController> ctrl;
};

// ============================================================
// Tests
// ============================================================

TEST_F(AnalysisControllerV2Test, NoHandle_EmitsAnalysisFailedSynchronously) {
    ctrl->onTrussHandleUpdated(0);

    EXPECT_CALL(facade, getTrussMutable(_)).Times(0);

    QSignalSpy spy{ctrl.get(), &AnalysisController::analysisFailed};
    ctrl->onAnalyzeRequested(AnalysisOptions{});

    ASSERT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.first().first().toString().isEmpty());
}

TEST_F(AnalysisControllerV2Test, AnalyzeRequested_EmitsAnalysisStarted) {
    EXPECT_CALL(facade, getTrussMutable(kHandle)).WillOnce(ReturnRef(localTruss));
    EXPECT_CALL(facade, analyze(_, _)).WillOnce(Return(Result<std::size_t>::Success(42)));

    QSignalSpy spyStarted{ctrl.get(), &AnalysisController::analysisStarted};
    QSignalSpy spyDone{ctrl.get(), &AnalysisController::analysisCompleted};

    ctrl->onAnalyzeRequested(AnalysisOptions{});

    // analysisStarted is emitted on the main thread synchronously before the thread starts
    ASSERT_EQ(spyStarted.count(), 1);

    // Wait for worker thread to complete (Qt::QueuedConnection delivers on next event loop tick)
    spyDone.wait(3000);
    EXPECT_EQ(spyDone.count(), 1);
}

TEST_F(AnalysisControllerV2Test, AnalyzeRequested_OnSuccess_EmitsAnalysisCompleted) {
    EXPECT_CALL(facade, getTrussMutable(kHandle)).WillOnce(ReturnRef(localTruss));
    EXPECT_CALL(facade, analyze(_, _)).WillOnce(Return(Result<std::size_t>::Success(77)));

    QSignalSpy spy{ctrl.get(), &AnalysisController::analysisCompleted};
    ctrl->onAnalyzeRequested(AnalysisOptions{});
    spy.wait(3000);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().first().value<std::size_t>(), std::size_t{77});
}

TEST_F(AnalysisControllerV2Test, AnalyzeRequested_OnFailure_EmitsAnalysisFailed) {
    EXPECT_CALL(facade, getTrussMutable(kHandle)).WillOnce(ReturnRef(localTruss));
    EXPECT_CALL(facade, analyze(_, _))
        .WillOnce(Return(Result<std::size_t>::Failure("singular matrix")));

    QSignalSpy spy{ctrl.get(), &AnalysisController::analysisFailed};
    ctrl->onAnalyzeRequested(AnalysisOptions{});
    spy.wait(3000);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_FALSE(spy.first().first().toString().isEmpty());
}

TEST_F(AnalysisControllerV2Test, CurrentResultsHandle_ReflectsLastSuccessfulResult) {
    EXPECT_CALL(facade, getTrussMutable(kHandle)).WillOnce(ReturnRef(localTruss));
    EXPECT_CALL(facade, analyze(_, _)).WillOnce(Return(Result<std::size_t>::Success(55)));

    QSignalSpy spy{ctrl.get(), &AnalysisController::analysisCompleted};
    ctrl->onAnalyzeRequested(AnalysisOptions{});
    spy.wait(3000);

    EXPECT_EQ(ctrl->currentResultsHandle(), std::size_t{55});
}
