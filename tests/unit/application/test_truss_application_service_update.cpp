/**
 * @file test_truss_application_service_update.cpp
 * @brief Unit tests for TrussApplicationService::updateNode and updateMember.
 * @version 3.0.0
 * @date 2026-03-07
 * @author Neil Taison Rigaud
 *
 * Test Coverage:
 * - updateNode() — valid handle, updates coordinates, preserves ID
 * - updateNode() — cascaded geometry: member length changes after node move
 * - updateNode() — invalid handle, non-existent node
 * - updateNode() — repeated updates never change the node's ID
 * - updateMember() — valid handle, updates material and section, preserves ID
 * - updateMember() — preserves start/end node connectivity
 * - updateMember() — invalid handle, non-existent member
 * - updateMember() — repeated updates never change the member's ID
 *
 * Architecture: Application Layer Tests (Domain-decoupled)
 * - Tests interact ONLY through Application Service public API
 * - No direct Domain layer imports (Domain types accessed via Application API)
 */

#include "../../../src/application/truss_application_service.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace truss::application;

using truss::core::MemberId;
using truss::core::NodeId;
using truss::core::Point2D;
using truss::core::SupportType;

// ============================================================================
// Fixture
// ============================================================================

class TrussApplicationServiceUpdateTest : public ::testing::Test {
protected:
    TrussApplicationService service;
    TrussHandle handle{0};

    // Pre-populated node/member IDs set up by SetUp().
    NodeId n1{0}, n2{0}, n3{0};
    MemberId m1{0}, m2{0}, m3{0};

    void SetUp() override {
        auto r = service.createTruss("UpdateTest");
        ASSERT_TRUE(r.success);
        handle = r.value;

        // Three nodes forming a simple triangular truss:
        //   n1(0,0) — n2(4,0)
        //         \   /
        //          n3(2,3)
        auto r1 = service.addNode(handle, Point2D{0.0, 0.0}, SupportType::Pinned);
        ASSERT_TRUE(r1.success);
        n1 = r1.value;

        auto r2 = service.addNode(handle, Point2D{4.0, 0.0}, SupportType::RollerX);
        ASSERT_TRUE(r2.success);
        n2 = r2.value;

        auto r3 = service.addNode(handle, Point2D{2.0, 3.0}, SupportType::Free);
        ASSERT_TRUE(r3.success);
        n3 = r3.value;

        // Three members — all three sides of the triangle
        const MaterialSpec mat{200e9, "Steel"};
        const SectionSpec sec{0.001, "Circular"};

        auto rm1 = service.addMember(handle, n1, n3, mat, sec);  // left leg
        ASSERT_TRUE(rm1.success);
        m1 = rm1.value;

        auto rm2 = service.addMember(handle, n2, n3, mat, sec);  // right leg
        ASSERT_TRUE(rm2.success);
        m2 = rm2.value;

        auto rm3 = service.addMember(handle, n1, n2, mat, sec);  // base
        ASSERT_TRUE(rm3.success);
        m3 = rm3.value;
    }
};

// ============================================================================
// updateNode — basic correctness
// ============================================================================

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_ValidHandle_ChangesCoordinatesSuccessfully) {
    NodeUpdateSpec spec{10.0, 5.0};
    auto result = service.updateNode(handle, n1, spec);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_ValidHandle_NewPositionReflectedInView) {
    NodeUpdateSpec spec{10.0, 5.0};
    ASSERT_TRUE(service.updateNode(handle, n1, spec).success);

    const auto& view = service.getTrussView(handle);
    bool found = false;
    for (const auto& nodeView : view.getNodeViews()) {
        if (nodeView.id == n1) {
            EXPECT_DOUBLE_EQ(nodeView.x, 10.0);
            EXPECT_DOUBLE_EQ(nodeView.y, 5.0);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Node not found in view after update";
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_ValidHandle_PreservesNodeId) {
    // The node ID must be identical before and after the update.
    const NodeId idBefore = n1;
    ASSERT_TRUE(service.updateNode(handle, n1, NodeUpdateSpec{7.0, 7.0}).success);

    const auto& view = service.getTrussView(handle);
    bool found = false;
    for (const auto& nodeView : view.getNodeViews()) {
        if (nodeView.id == idBefore) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Node ID changed after updateNode — identity not preserved";
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_ValidHandle_MemberLengthUpdatesAfterNodeMove) {
    // Move n3 from (2,3) straight above n1 at (0,6). m1 (n1→n3) new length = 6.0.
    ASSERT_TRUE(service.updateNode(handle, n3, NodeUpdateSpec{0.0, 6.0}).success);

    const auto& view = service.getTrussView(handle);
    for (const auto& memberView : view.getMemberViews()) {
        if (memberView.id == m1) {
            EXPECT_NEAR(memberView.length, 6.0, 1e-6);
            return;
        }
    }
    FAIL() << "Member m1 not found in view";
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_MultipleConsecutiveUpdates_NodeIdUnchanged) {
    const NodeId originalId = n2;

    ASSERT_TRUE(service.updateNode(handle, n2, NodeUpdateSpec{1.0, 0.0}).success);
    ASSERT_TRUE(service.updateNode(handle, n2, NodeUpdateSpec{2.0, 0.0}).success);
    ASSERT_TRUE(service.updateNode(handle, n2, NodeUpdateSpec{3.0, 0.0}).success);

    // Node must still exist with the original ID
    const auto& view = service.getTrussView(handle);
    bool found = false;
    for (const auto& nodeView : view.getNodeViews())
        if (nodeView.id == originalId) {
            found = true;
            break;
        }
    EXPECT_TRUE(found) << "Node ID drifted across consecutive updateNode calls";
}

// ============================================================================
// updateNode — failure paths
// ============================================================================

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_InvalidHandle_ReturnsFailure) {
    constexpr TrussHandle badHandle = 99999;
    auto result = service.updateNode(badHandle, n1, NodeUpdateSpec{1.0, 1.0});
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateNode_NonExistentNodeId_ReturnsFailure) {
    const NodeId nonExistent{99999};
    auto result = service.updateNode(handle, nonExistent, NodeUpdateSpec{0.0, 0.0});
    EXPECT_FALSE(result.success);
}

// ============================================================================
// updateMember — basic correctness
// ============================================================================

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_ValidHandle_ReturnsSuccess) {
    MemberUpdateSpec spec{MaterialSpec{69e9, "Aluminum"}, SectionSpec{0.005, "Box"}};
    auto result = service.updateMember(handle, m1, spec);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.value);
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_ValidHandle_NewMaterialReflectedInView) {
    const double newE = 70e9;
    MemberUpdateSpec spec{MaterialSpec{newE, "Aluminum"}, SectionSpec{0.002, "Custom"}};
    ASSERT_TRUE(service.updateMember(handle, m1, spec).success);

    const auto& view = service.getTrussView(handle);
    for (const auto& mv : view.getMemberViews()) {
        if (mv.id == m1) {
            EXPECT_NEAR(mv.youngModulus, newE, 1.0);
            return;
        }
    }
    FAIL() << "Member m1 not found in view after updateMember";
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_ValidHandle_PreservesMemberId) {
    const MemberId idBefore = m1;
    ASSERT_TRUE(
        service
            .updateMember(
                handle, m1, MemberUpdateSpec{MaterialSpec{69e9, "Al"}, SectionSpec{0.003, "Rect"}})
            .success);

    const auto& view = service.getTrussView(handle);
    bool found = false;
    for (const auto& mv : view.getMemberViews())
        if (mv.id == idBefore) {
            found = true;
            break;
        }
    EXPECT_TRUE(found) << "Member ID changed after updateMember — identity not preserved";
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_ValidHandle_PreservesNodeConnectivity) {
    // After updating m1, its start and end node IDs must be unchanged.
    NodeId startBefore{0}, endBefore{0};
    {
        const auto& view = service.getTrussView(handle);
        for (const auto& mv : view.getMemberViews()) {
            if (mv.id == m1) {
                startBefore = mv.startNodeId;
                endBefore = mv.endNodeId;
                break;
            }
        }
    }
    ASSERT_NE(startBefore, NodeId{0});

    ASSERT_TRUE(service
                    .updateMember(handle,
                                  m1,
                                  MemberUpdateSpec{MaterialSpec{200e9, "Steel"},
                                                   SectionSpec{0.002, "Custom"}})
                    .success);

    const auto& view = service.getTrussView(handle);
    for (const auto& mv : view.getMemberViews()) {
        if (mv.id == m1) {
            EXPECT_EQ(mv.startNodeId, startBefore);
            EXPECT_EQ(mv.endNodeId, endBefore);
            return;
        }
    }
    FAIL() << "Member m1 not found in view after update";
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_MultipleUpdates_MemberIdNeverChanges) {
    const MemberId originalId = m2;

    ASSERT_TRUE(
        service
            .updateMember(
                handle, m2, MemberUpdateSpec{MaterialSpec{200e9, "Steel"}, SectionSpec{0.001, "A"}})
            .success);
    ASSERT_TRUE(
        service
            .updateMember(handle,
                          m2,
                          MemberUpdateSpec{MaterialSpec{69e9, "Aluminum"}, SectionSpec{0.002, "B"}})
            .success);
    ASSERT_TRUE(service
                    .updateMember(
                        handle,
                        m2,
                        MemberUpdateSpec{MaterialSpec{110e9, "Titanium"}, SectionSpec{0.003, "C"}})
                    .success);

    const auto& view = service.getTrussView(handle);
    bool found = false;
    for (const auto& mv : view.getMemberViews())
        if (mv.id == originalId) {
            found = true;
            break;
        }
    EXPECT_TRUE(found) << "Member ID drifted across consecutive updateMember calls";
}

// ============================================================================
// updateMember — failure paths
// ============================================================================

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_InvalidHandle_ReturnsFailure) {
    constexpr TrussHandle badHandle = 99999;
    auto result = service.updateMember(
        badHandle, m1, MemberUpdateSpec{MaterialSpec{200e9, "Steel"}, SectionSpec{0.001, "A"}});
    EXPECT_FALSE(result.success);
}

TEST_F(TrussApplicationServiceUpdateTest, UpdateMember_NonExistentMemberId_ReturnsFailure) {
    const MemberId nonExistent{99999};
    auto result = service.updateMember(
        handle,
        nonExistent,
        MemberUpdateSpec{MaterialSpec{200e9, "Steel"}, SectionSpec{0.001, "A"}});
    EXPECT_FALSE(result.success);
}
