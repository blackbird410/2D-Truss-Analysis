/**
 * @file test_canvas_controller.cpp
 * @brief Unit tests for CanvasController (Phase 5).
 *
 * Verifies:
 *  - onNodeDropRequested calls facade.addNode and emits trussModified on success.
 *  - onNodeDropRequested emits operationFailed when facade returns an error.
 *  - onMemberDrawRequested calls facade.addMember and emits trussModified on success.
 *  - onNodeDeleteRequested calls facade.removeNode.
 *  - onMemberDeleteRequested calls facade.removeMember.
 *  - No active truss handle (0) causes operationFailed without touching facade.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "gui/controllers/canvas_controller.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::gui::ctrl;
using namespace truss::core;
using truss::application::Result;
using truss::test::MockTrussAnalysisFacade;
using ::testing::_;
using ::testing::Return;

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
}  // namespace

// ============================================================
// Fixture
// ============================================================

class CanvasControllerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ensureQApp();
        ctrl = std::make_unique<CanvasController>(facade);
        ctrl->onTrussHandleUpdated(kHandle);
    }

    void TearDown() override { ctrl.reset(); }

    static constexpr std::size_t kHandle = 42;

    ::testing::NiceMock<MockTrussAnalysisFacade> facade;
    std::unique_ptr<CanvasController>             ctrl;
};

// ============================================================
// Tests — Node operations
// ============================================================

TEST_F(CanvasControllerTest, NodeDrop_Success_CallsAddNodeAndEmitsModified)
{
    EXPECT_CALL(facade, addNode(kHandle, _, _))
        .WillOnce(Return(Result<NodeId>::Success(NodeId{1})));

    QSignalSpy spy{ctrl.get(), &CanvasController::trussModified};
    ctrl->onNodeDropRequested(Point2D{1.0, 2.0}, SupportType::Free);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().first().value<std::size_t>(), kHandle);
}

TEST_F(CanvasControllerTest, NodeDrop_Failure_EmitsOperationFailed)
{
    EXPECT_CALL(facade, addNode(kHandle, _, _))
        .WillOnce(Return(Result<NodeId>::Failure("duplicate position")));

    QSignalSpy spyFail{ctrl.get(), &CanvasController::operationFailed};
    QSignalSpy spyMod{ctrl.get(), &CanvasController::trussModified};
    ctrl->onNodeDropRequested(Point2D{1.0, 2.0}, SupportType::Free);

    EXPECT_EQ(spyMod.count(), 0);
    ASSERT_EQ(spyFail.count(), 1);
    EXPECT_FALSE(spyFail.first().first().toString().isEmpty());
}

TEST_F(CanvasControllerTest, NodeDrop_NoHandle_EmitsOperationFailedWithoutCallingFacade)
{
    ctrl->onTrussHandleUpdated(0);  // reset handle to 0

    EXPECT_CALL(facade, addNode(_, _, _)).Times(0);

    QSignalSpy spy{ctrl.get(), &CanvasController::operationFailed};
    ctrl->onNodeDropRequested(Point2D{0, 0}, SupportType::Free);

    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// Tests — Member operations
// ============================================================

TEST_F(CanvasControllerTest, MemberDraw_Success_CallsAddMemberAndEmitsModified)
{
    EXPECT_CALL(facade, addMember(kHandle, NodeId{1}, NodeId{2}, _, _))
        .WillOnce(Return(Result<MemberId>::Success(MemberId{1})));

    QSignalSpy spy{ctrl.get(), &CanvasController::trussModified};
    ctrl->onMemberDrawRequested(NodeId{1}, NodeId{2});

    ASSERT_EQ(spy.count(), 1);
}

TEST_F(CanvasControllerTest, MemberDraw_Failure_EmitsOperationFailed)
{
    EXPECT_CALL(facade, addMember(_, _, _, _, _))
        .WillOnce(Return(Result<MemberId>::Failure("invalid nodes")));

    QSignalSpy spy{ctrl.get(), &CanvasController::operationFailed};
    ctrl->onMemberDrawRequested(NodeId{1}, NodeId{999});

    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// Tests — Delete operations
// ============================================================

TEST_F(CanvasControllerTest, NodeDelete_Success_CallsRemoveNodeAndEmitsModified)
{
    EXPECT_CALL(facade, removeNode(kHandle, NodeId{1}))
        .WillOnce(Return(Result<bool>::Success(true)));

    QSignalSpy spy{ctrl.get(), &CanvasController::trussModified};
    ctrl->onNodeDeleteRequested(NodeId{1});

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CanvasControllerTest, MemberDelete_Success_CallsRemoveMemberAndEmitsModified)
{
    EXPECT_CALL(facade, removeMember(kHandle, MemberId{3}))
        .WillOnce(Return(Result<bool>::Success(true)));

    QSignalSpy spy{ctrl.get(), &CanvasController::trussModified};
    ctrl->onMemberDeleteRequested(MemberId{3});

    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// Tests — Handle update
// ============================================================

TEST_F(CanvasControllerTest, TrussHandleUpdated_ChangesActiveHandle)
{
    ctrl->onTrussHandleUpdated(99);

    EXPECT_CALL(facade, addNode(99, _, _))
        .WillOnce(Return(Result<NodeId>::Success(NodeId{5})));

    QSignalSpy spy{ctrl.get(), &CanvasController::trussModified};
    ctrl->onNodeDropRequested(Point2D{0, 0}, SupportType::Free);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.first().first().value<std::size_t>(), std::size_t{99});
}
