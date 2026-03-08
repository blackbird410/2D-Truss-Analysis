/**
 * @file test_inspector_controller.cpp
 * @brief Unit tests for InspectorController (Phase 5).
 *
 * Verifies:
 *  - Node selection emits nodeViewReady with the matching NodeView.
 *  - Member selection emits memberViewReady with the matching MemberView.
 *  - Selection cleared emits selectionCleared signal.
 *  - Support change forwards to facade.setNodeSupport and emits trussModified.
 *  - Load change forwards to facade.applyNodeLoad and emits trussModified.
 *  - Zero load calls facade.clearNodeLoad instead of applyNodeLoad.
 *  - No truss handle skips facade calls and emits operationFailed.
 *
 * @author Neil Taison Rigaud
 * @version 3.0.0
 * @date 2026-03-04
 */

#include "core/interfaces/itruss_view.hpp"
#include "core/model/types.hpp"
#include "gui/controllers/inspector_controller.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace truss::gui::ctrl;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using truss::application::Result;
using truss::core::Force2D;
using truss::core::MemberId;
using truss::core::NodeId;
using truss::core::SupportType;
using truss::core::interfaces::ITrussView;
using truss::core::interfaces::MemberView;
using truss::core::interfaces::NodeView;
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

// ============================================================
// Minimal ITrussView stub — 3-node, 2-member truss
// ============================================================

class StubTrussView final : public ITrussView {
public:
    StubTrussView() {
        NodeView n1;
        n1.id = 1;
        n1.x = 0.0;
        n1.y = 0.0;
        n1.support = SupportType::Pinned;
        m_nodes.push_back(n1);

        NodeView n2;
        n2.id = 2;
        n2.x = 4.0;
        n2.y = 0.0;
        n2.support = SupportType::RollerX;
        m_nodes.push_back(n2);

        NodeView n3;
        n3.id = 3;
        n3.x = 2.0;
        n3.y = 3.0;
        n3.support = SupportType::Free;
        n3.fx = 0.0;
        n3.fy = -50000.0;
        m_nodes.push_back(n3);

        MemberView m1;
        m1.id = 1;
        m1.startNodeId = 1;
        m1.endNodeId = 3;
        m1.youngModulus = 200e9;
        m1.area = 0.001;
        m1.length = 3.606;
        m1.angle = 0.98;
        m_members.push_back(m1);

        MemberView m2;
        m2.id = 2;
        m2.startNodeId = 2;
        m2.endNodeId = 3;
        m2.youngModulus = 200e9;
        m2.area = 0.001;
        m2.length = 3.606;
        m2.angle = 2.16;
        m_members.push_back(m2);
    }

    const std::string& getName() const override { return m_name; }

    std::vector<NodeView> getNodeViews() const override { return m_nodes; }
    std::size_t getNodeCount() const override { return m_nodes.size(); }
    std::vector<MemberView> getMemberViews() const override { return m_members; }
    std::size_t getMemberCount() const override { return m_members.size(); }

    std::size_t getTotalDofs() const override { return getNodeCount() * 2; }
    std::size_t getFreeDofs() const override { return 2; }
    std::size_t getConstrainedDofs() const override { return getTotalDofs() - getFreeDofs(); }

private:
    std::string m_name{"StubTruss"};
    std::vector<NodeView> m_nodes;
    std::vector<MemberView> m_members;
};

}  // namespace

// ============================================================
// Fixture
// ============================================================

class InspectorControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ensureQApp();
        ctrl = std::make_unique<InspectorController>(facade);
        ctrl->onTrussHandleUpdated(kHandle);
    }
    void TearDown() override { ctrl.reset(); }

    static constexpr std::size_t kHandle = 7;

    NiceMock<MockTrussAnalysisFacade> facade;
    StubTrussView stubView;
    std::unique_ptr<InspectorController> ctrl;
};

// ============================================================
// Selection → view-ready signals
// ============================================================

TEST_F(InspectorControllerTest, NodeSelection_EmitsNodeViewReadyWithCorrectId) {
    EXPECT_CALL(facade, getTrussView(kHandle))
        .WillOnce(ReturnRef(static_cast<const ITrussView&>(stubView)));

    QSignalSpy spy{ctrl.get(), &InspectorController::nodeViewReady};
    ctrl->onNodeSelectionChanged(NodeId{1});

    ASSERT_EQ(spy.count(), 1);
    auto emitted = spy.first().first().value<NodeView>();
    EXPECT_EQ(emitted.id, NodeId{1});
}

TEST_F(InspectorControllerTest, MemberSelection_EmitsMemberViewReadyWithCorrectId) {
    EXPECT_CALL(facade, getTrussView(kHandle))
        .WillOnce(ReturnRef(static_cast<const ITrussView&>(stubView)));

    QSignalSpy spy{ctrl.get(), &InspectorController::memberViewReady};
    ctrl->onMemberSelectionChanged(MemberId{2});

    ASSERT_EQ(spy.count(), 1);
    auto emitted = spy.first().first().value<MemberView>();
    EXPECT_EQ(emitted.id, MemberId{2});
}

TEST_F(InspectorControllerTest, SelectionCleared_EmitsSelectionClearedSignal) {
    QSignalSpy spy{ctrl.get(), &InspectorController::selectionCleared};
    ctrl->onSelectionCleared();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(InspectorControllerTest, NodeSelection_UnknownId_DoesNotEmitAnySignal) {
    EXPECT_CALL(facade, getTrussView(kHandle))
        .WillOnce(ReturnRef(static_cast<const ITrussView&>(stubView)));

    QSignalSpy spyReady{ctrl.get(), &InspectorController::nodeViewReady};
    QSignalSpy spyFail{ctrl.get(), &InspectorController::operationFailed};
    ctrl->onNodeSelectionChanged(NodeId{999});

    // Implementation silently ignores unknown IDs (stale selection is not an error).
    EXPECT_EQ(spyReady.count(), 0);
    EXPECT_EQ(spyFail.count(), 0);
}

// ============================================================
// Support & load edits → facade calls
// ============================================================

TEST_F(InspectorControllerTest, SupportChange_CallsSetNodeSupportAndEmitsModified) {
    EXPECT_CALL(facade, setNodeSupport(kHandle, NodeId{1}, SupportType::RollerY))
        .WillOnce(Return(Result<bool>::Success(true)));

    QSignalSpy spy{ctrl.get(), &InspectorController::trussModified};
    ctrl->onSupportChangeRequested(NodeId{1}, SupportType::RollerY);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(InspectorControllerTest, LoadChange_NonZero_CallsApplyNodeLoadAndEmitsModified) {
    EXPECT_CALL(facade, applyNodeLoad(kHandle, NodeId{3}, _))
        .WillOnce(Return(Result<bool>::Success(true)));

    QSignalSpy spy{ctrl.get(), &InspectorController::trussModified};
    ctrl->onLoadChangeRequested(NodeId{3}, Force2D{0.0, -50000.0});

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(InspectorControllerTest, LoadChange_ZeroLoad_CallsClearNodeLoad) {
    EXPECT_CALL(facade, clearNodeLoad(kHandle, NodeId{3}))
        .WillOnce(Return(Result<bool>::Success(true)));
    EXPECT_CALL(facade, applyNodeLoad(_, _, _)).Times(0);

    QSignalSpy spy{ctrl.get(), &InspectorController::trussModified};
    ctrl->onLoadChangeRequested(NodeId{3}, Force2D{0.0, 0.0});

    EXPECT_EQ(spy.count(), 1);
}

// ============================================================
// No truss handle
// ============================================================

TEST_F(InspectorControllerTest, SelectionWithNoHandle_IgnoresSilently) {
    ctrl->onTrussHandleUpdated(0);

    EXPECT_CALL(facade, getTrussView(_)).Times(0);

    // No handle = no truss loaded. Selection changes are silently ignored.
    QSignalSpy spyReady{ctrl.get(), &InspectorController::nodeViewReady};
    QSignalSpy spyFail{ctrl.get(), &InspectorController::operationFailed};
    ctrl->onNodeSelectionChanged(NodeId{1});

    EXPECT_EQ(spyReady.count(), 0);
    EXPECT_EQ(spyFail.count(), 0);
}
// ============================================================
// updateNode — node position change
// ============================================================

TEST_F(InspectorControllerTest, NodePositionChange_CallsUpdateNodeAndEmitsModified) {
    using truss::application::NodeUpdateSpec;
    using truss::core::Point2D;

    EXPECT_CALL(facade,
                updateNode(kHandle,
                           NodeId{2},
                           testing::Field(&NodeUpdateSpec::x, testing::DoubleEq(5.0))))
        .WillOnce(Return(Result<bool>::Success(true)));

    QSignalSpy spyMod{ctrl.get(), &InspectorController::trussModified};
    QSignalSpy spyFail{ctrl.get(), &InspectorController::operationFailed};

    ctrl->onNodePositionChangeRequested(NodeId{2}, Point2D{5.0, 3.0});

    EXPECT_EQ(spyMod.count(), 1);
    EXPECT_EQ(spyFail.count(), 0);
}

TEST_F(InspectorControllerTest, NodePositionChange_FacadeFailure_EmitsOperationFailed) {
    using truss::core::Point2D;

    EXPECT_CALL(facade, updateNode(kHandle, NodeId{1}, _))
        .WillOnce(Return(Result<bool>::Failure("node not found")));

    QSignalSpy spyMod{ctrl.get(), &InspectorController::trussModified};
    QSignalSpy spyFail{ctrl.get(), &InspectorController::operationFailed};

    ctrl->onNodePositionChangeRequested(NodeId{1}, Point2D{99.0, 99.0});

    EXPECT_EQ(spyMod.count(), 0);
    EXPECT_EQ(spyFail.count(), 1);
}

TEST_F(InspectorControllerTest, NodePositionChange_NoHandle_EmitsOperationFailed) {
    using truss::core::Point2D;

    ctrl->onTrussHandleUpdated(0);
    EXPECT_CALL(facade, updateNode(_, _, _)).Times(0);

    QSignalSpy spyFail{ctrl.get(), &InspectorController::operationFailed};
    ctrl->onNodePositionChangeRequested(NodeId{1}, Point2D{1.0, 2.0});

    EXPECT_EQ(spyFail.count(), 1);
}

// ============================================================
// updateMember — property change (no remove + re-add)
// ============================================================

TEST_F(InspectorControllerTest,
       MemberPropertiesChange_CallsUpdateMemberNotRemoveAdd) {
    using truss::application::MaterialSpec;
    using truss::application::MemberUpdateSpec;
    using truss::application::SectionSpec;

    const MaterialSpec mat{200e9, "Steel"};
    const SectionSpec  sec{0.002, "Custom"};

    // updateMember must be called exactly once; removeMember / addMember must NOT.
    EXPECT_CALL(facade, updateMember(kHandle, MemberId{1}, _))
        .WillOnce(Return(Result<bool>::Success(true)));
    EXPECT_CALL(facade, removeMember(_, _)).Times(0);
    EXPECT_CALL(facade, addMember(_, _, _, _, _)).Times(0);

    QSignalSpy spyMod{ctrl.get(), &InspectorController::trussModified};
    ctrl->onMemberPropertiesChangeRequested(MemberId{1}, mat, sec);

    EXPECT_EQ(spyMod.count(), 1);
}

TEST_F(InspectorControllerTest,
       MemberPropertiesChange_PreservesMemberIdAfterUpdate) {
    // After a successful updateMember the ID passed in must be unchanged.
    // This verifies the controller does NOT modify the member's ID.
    using truss::application::MaterialSpec;
    using truss::application::MemberUpdateSpec;
    using truss::application::SectionSpec;

    const MemberId originalId{2};
    EXPECT_CALL(facade,
                updateMember(kHandle,
                             originalId,  // same ID must be forwarded
                             _))
        .WillOnce(Return(Result<bool>::Success(true)));

    QSignalSpy spyMod{ctrl.get(), &InspectorController::trussModified};
    ctrl->onMemberPropertiesChangeRequested(originalId,
                                            MaterialSpec{69e9, "Aluminum"},
                                            SectionSpec{0.001, "Circular"});
    EXPECT_EQ(spyMod.count(), 1);
}

TEST_F(InspectorControllerTest,
       MemberPropertiesChange_FacadeFailure_EmitsOperationFailed) {
    using truss::application::MaterialSpec;
    using truss::application::SectionSpec;

    EXPECT_CALL(facade, updateMember(kHandle, MemberId{1}, _))
        .WillOnce(Return(Result<bool>::Failure("member not found")));

    QSignalSpy spyMod{ctrl.get(), &InspectorController::trussModified};
    QSignalSpy spyFail{ctrl.get(), &InspectorController::operationFailed};

    ctrl->onMemberPropertiesChangeRequested(MemberId{1},
                                            MaterialSpec{200e9, "Steel"},
                                            SectionSpec{0.001, "Custom"});
    EXPECT_EQ(spyMod.count(), 0);
    EXPECT_EQ(spyFail.count(), 1);
}

TEST_F(InspectorControllerTest,
       MemberPropertiesChange_RepeatedEdits_KeepsSameMemberId) {
    // Simulate two consecutive "Apply Changes" clicks on the same member.
    // Both calls must target the same member ID (no ID drift).
    using truss::application::MaterialSpec;
    using truss::application::SectionSpec;

    const MemberId targetId{1};

    EXPECT_CALL(facade, updateMember(kHandle, targetId, _))
        .Times(2)
        .WillRepeatedly(Return(Result<bool>::Success(true)));

    QSignalSpy spyMod{ctrl.get(), &InspectorController::trussModified};

    ctrl->onMemberPropertiesChangeRequested(targetId, MaterialSpec{200e9, "Steel"},
                                            SectionSpec{0.001, "Custom"});
    ctrl->onMemberPropertiesChangeRequested(targetId, MaterialSpec{69e9, "Aluminum"},
                                            SectionSpec{0.002, "Custom"});

    EXPECT_EQ(spyMod.count(), 2);
}

