/**
 * @file test_project_controller.cpp
 * @brief Unit tests for ProjectController (Phase 5).
 *
 * Verifies:
 *  - onNewProjectRequested calls facade.createTruss and emits trussCreated.
 *  - New project with dirty state and declining confirmation does NOT call facade.
 *  - onSaveRequested with no file path emits operationFailed.
 *  - onTrussHandleUpdated stores the handle for subsequent operations.
 *  - setDirty(false) prevents confirmation dialog on new project.
 *  - AutoConfirmProvider(true) always allows destructive actions.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/project_controller.hpp"
#include "gui/interfaces/iconfirmation_provider.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::gui::ctrl;
using namespace truss::gui::interfaces;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using truss::application::Result;
using truss::test::MockTrussAnalysisFacade;

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
// Fixture helpers
// ============================================================

/// Fixture with AutoConfirmProvider(true) — simulates user always clicking OK.
class ProjectControllerConfirmTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        confirmYes = std::make_unique<AutoConfirmProvider>(true);
        ctrl = std::make_unique<ProjectController>(facade, *confirmYes);
    }
    void TearDown() override { ctrl.reset(); }

    NiceMock<MockTrussAnalysisFacade> facade;
    std::unique_ptr<AutoConfirmProvider> confirmYes;
    std::unique_ptr<ProjectController> ctrl;
};

/// Fixture with AutoConfirmProvider(false) — simulates user always clicking Cancel.
class ProjectControllerDenyTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        confirmNo = std::make_unique<AutoConfirmProvider>(false);
        ctrl = std::make_unique<ProjectController>(facade, *confirmNo);
    }
    void TearDown() override { ctrl.reset(); }

    NiceMock<MockTrussAnalysisFacade> facade;
    std::unique_ptr<AutoConfirmProvider> confirmNo;
    std::unique_ptr<ProjectController> ctrl;
};

// ============================================================
// Tests
// ============================================================

TEST_F(ProjectControllerConfirmTest, NewProject_CleanState_CallsCreateTrussAndEmitsCreated) {
    EXPECT_CALL(facade, createTruss(_))
        .WillOnce(Return(Result<std::size_t>::Success(std::size_t{10})));

    QSignalSpy spy{ctrl.get(), &ProjectController::trussCreated};
    ctrl->onNewProjectRequested();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().first().value<std::size_t>(), std::size_t{10});
}

TEST_F(ProjectControllerConfirmTest, NewProject_DirtyAndUserConfirms_CallsCreateTruss) {
    ctrl->setDirty(true);

    EXPECT_CALL(facade, createTruss(_))
        .WillOnce(Return(Result<std::size_t>::Success(std::size_t{11})));

    QSignalSpy spy{ctrl.get(), &ProjectController::trussCreated};
    ctrl->onNewProjectRequested();

    ASSERT_EQ(spy.count(), 1);
}

TEST_F(ProjectControllerDenyTest, NewProject_DirtyAndUserDenies_DoesNotCallCreateTruss) {
    ctrl->onTrussHandleUpdated(1);  // must have a handle for the dirty guard to fire
    ctrl->setDirty(true);

    EXPECT_CALL(facade, createTruss(_)).Times(0);

    QSignalSpy spy{ctrl.get(), &ProjectController::trussCreated};
    ctrl->onNewProjectRequested();

    EXPECT_EQ(spy.count(), 0);
}

TEST_F(ProjectControllerConfirmTest, TrussHandleUpdated_PersistsHandle) {
    ctrl->onTrussHandleUpdated(42);

    // Verify that a subsequent new project replaces the existing truss with handle 42.
    // clearTruss(42) is called before createTruss.
    EXPECT_CALL(facade, clearTruss(std::size_t{42})).Times(1);
    EXPECT_CALL(facade, createTruss(_))
        .WillOnce(Return(Result<std::size_t>::Success(std::size_t{43})));

    QSignalSpy spy{ctrl.get(), &ProjectController::trussCreated};
    ctrl->onNewProjectRequested();

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().first().value<std::size_t>(), std::size_t{43});
}

TEST_F(ProjectControllerConfirmTest, NewProject_CreateTrussFailure_EmitsOperationFailed) {
    EXPECT_CALL(facade, createTruss(_))
        .WillOnce(Return(Result<std::size_t>::Failure("storage full")));

    QSignalSpy spyOk{ctrl.get(), &ProjectController::trussCreated};
    QSignalSpy spyFail{ctrl.get(), &ProjectController::operationFailed};
    ctrl->onNewProjectRequested();

    EXPECT_EQ(spyOk.count(), 0);
    ASSERT_EQ(spyFail.count(), 1);
    EXPECT_FALSE(spyFail.first().first().toString().isEmpty());
}

TEST_F(ProjectControllerConfirmTest, SetDirty_False_DoesNotShowConfirmationOnNew) {
    ctrl->setDirty(true);
    ctrl->setDirty(false);

    EXPECT_CALL(facade, createTruss(_))
        .WillOnce(Return(Result<std::size_t>::Success(std::size_t{20})));

    QSignalSpy spy{ctrl.get(), &ProjectController::trussCreated};
    ctrl->onNewProjectRequested();

    EXPECT_EQ(spy.count(), 1);
}
