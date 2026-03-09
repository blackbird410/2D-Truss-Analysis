/**
 * @file test_member_branches.cpp
 * @brief Branch coverage tests for Member class.
 * @version 3.0.0
 *
 * Targets previously uncovered branches in member.cpp:
 * - getOtherNode() throw paths
 * - getSlope() vertical branch
 * - intersectsWith() parallel and non-intersecting paths
 * - getIntersectionPoint() parallel throw
 * - isParallelTo() / isPerpendicularTo()
 * - getMidpoint(), getTransformationMatrix(), getGlobalDofIndices()
 * - operator== / operator!=
 * - updateResults() tension/compression and yielded branches
 */

#include "model/member.hpp"
#include "model/node.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <memory>

using namespace truss::core;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<Node> makeNode(NodeId id, double x, double y) {
    return std::make_shared<Node>(id, Point2D(x, y));
}

static Member makeMember(MemberId id, std::shared_ptr<Node> n1, std::shared_ptr<Node> n2) {
    return Member(id, n1, n2);
}

// ---------------------------------------------------------------------------
// getOtherNode(const Node&) – throw path
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetOtherNodeByRef_ThrowsWhenNotConnected) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    auto n3 = makeNode(3, 2.0, 0.0);  // unrelated node

    Member m = makeMember(1, n1, n2);
    EXPECT_THROW(m.getOtherNode(*n3), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// getOtherNode(NodeId) – throw path
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetOtherNodeById_ThrowsWhenNotConnected) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    EXPECT_THROW(m.getOtherNode(NodeId{99}), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// getOtherNode – success paths (startNode → returns endNode, and vice versa)
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetOtherNodeByRef_ReturnsEndNode) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    EXPECT_EQ(m.getOtherNode(*n1), n2);
    EXPECT_EQ(m.getOtherNode(*n2), n1);
}

TEST(MemberBranchTest, GetOtherNodeById_ReturnsCorrectNode) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    EXPECT_EQ(m.getOtherNode(NodeId{1}), n2);
    EXPECT_EQ(m.getOtherNode(NodeId{2}), n1);
}

// ---------------------------------------------------------------------------
// getSlope() – vertical member (dir.x == 0) → returns infinity
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetSlope_VerticalMember_ReturnsInfinity) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 0.0, 5.0);  // vertical
    Member m = makeMember(1, n1, n2);

    EXPECT_TRUE(std::isinf(m.getSlope()));
}

TEST(MemberBranchTest, GetSlope_HorizontalMember_ReturnsZero) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 4.0, 0.0);  // horizontal
    Member m = makeMember(1, n1, n2);

    EXPECT_DOUBLE_EQ(m.getSlope(), 0.0);
}

// ---------------------------------------------------------------------------
// isParallelTo()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, IsParallelTo_HorizontalMembers_ReturnsTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 2.0, 0.0);
    auto n3 = makeNode(3, 0.0, 1.0);
    auto n4 = makeNode(4, 4.0, 1.0);

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_TRUE(m1.isParallelTo(m2));
}

TEST(MemberBranchTest, IsParallelTo_NonParallelMembers_ReturnsFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 2.0, 0.0);
    auto n3 = makeNode(3, 0.0, 0.0);
    auto n4 = makeNode(4, 1.0, 2.0);

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_FALSE(m1.isParallelTo(m2));
}

// ---------------------------------------------------------------------------
// isPerpendicularTo()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, IsPerpendicularTo_ReturnsTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);  // horizontal
    auto n3 = makeNode(3, 0.5, 0.0);
    auto n4 = makeNode(4, 0.5, 1.0);  // vertical

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_TRUE(m1.isPerpendicularTo(m2));
}

TEST(MemberBranchTest, IsPerpendicularTo_ReturnsFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    auto n3 = makeNode(3, 0.0, 0.0);
    auto n4 = makeNode(4, 1.0, 1.0);  // 45 degrees

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_FALSE(m1.isPerpendicularTo(m2));
}

// ---------------------------------------------------------------------------
// getMidpoint()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetMidpoint_ReturnsCorrectPoint) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 4.0, 2.0);
    Member m = makeMember(1, n1, n2);

    auto mid = m.getMidpoint();
    EXPECT_DOUBLE_EQ(mid.x, 2.0);
    EXPECT_DOUBLE_EQ(mid.y, 1.0);
}

// ---------------------------------------------------------------------------
// getGlobalDofIndices()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetGlobalDofIndices_ReturnsFourIndices) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    n1->setDofX(0);
    n1->setDofY(1);
    n2->setDofX(2);
    n2->setDofY(3);
    Member m = makeMember(1, n1, n2);

    auto dofs = m.getGlobalDofIndices();
    ASSERT_EQ(dofs.size(), 4u);
    EXPECT_EQ(dofs[0], 0u);
    EXPECT_EQ(dofs[1], 1u);
    EXPECT_EQ(dofs[2], 2u);
    EXPECT_EQ(dofs[3], 3u);
}

// ---------------------------------------------------------------------------
// getTransformationMatrix()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetTransformationMatrix_HorizontalMember_IdentityLike) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);  // unit horizontal
    Member m = makeMember(1, n1, n2);

    auto T = m.getTransformationMatrix();
    EXPECT_NEAR(T(0, 0), 1.0, 1e-9);
    EXPECT_NEAR(T(0, 1), 0.0, 1e-9);
    EXPECT_NEAR(T(1, 0), 0.0, 1e-9);
    EXPECT_NEAR(T(1, 1), 1.0, 1e-9);
}

TEST(MemberBranchTest, GetTransformationMatrix_VerticalMember) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 0.0, 1.0);  // unit vertical
    Member m = makeMember(1, n1, n2);

    auto T = m.getTransformationMatrix();
    // unitVec = (0, 1): matrix = [[0,1],[-1,0]]
    EXPECT_NEAR(T(0, 0), 0.0, 1e-9);
    EXPECT_NEAR(T(0, 1), 1.0, 1e-9);
    EXPECT_NEAR(T(1, 0), -1.0, 1e-9);
    EXPECT_NEAR(T(1, 1), 0.0, 1e-9);
}

// ---------------------------------------------------------------------------
// intersectsWith()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, IntersectsWith_CrossingMembers_ReturnsTrue) {
    // X-cross: (0,0)-(2,2) and (0,2)-(2,0)
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 2.0, 2.0);
    auto n3 = makeNode(3, 0.0, 2.0);
    auto n4 = makeNode(4, 2.0, 0.0);

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_TRUE(m1.intersectsWith(m2));
}

TEST(MemberBranchTest, IntersectsWith_ParallelMembers_ReturnsFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 2.0, 0.0);
    auto n3 = makeNode(3, 0.0, 1.0);
    auto n4 = makeNode(4, 2.0, 1.0);  // parallel, offset

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_FALSE(m1.intersectsWith(m2));
}

TEST(MemberBranchTest, IntersectsWith_NonIntersectingSegments_ReturnsFalse) {
    // Lines would intersect if extended, but segments don't overlap
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    auto n3 = makeNode(3, 5.0, -1.0);
    auto n4 = makeNode(4, 5.0, 1.0);  // vertical line far right

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_FALSE(m1.intersectsWith(m2));
}

// ---------------------------------------------------------------------------
// getIntersectionPoint()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetIntersectionPoint_CrossingMembers_CorrectPoint) {
    // (0,0)-(2,2) and (0,2)-(2,0) intersect at (1,1)
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 2.0, 2.0);
    auto n3 = makeNode(3, 0.0, 2.0);
    auto n4 = makeNode(4, 2.0, 0.0);

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    auto pt = m1.getIntersectionPoint(m2);
    EXPECT_NEAR(pt.x, 1.0, 1e-9);
    EXPECT_NEAR(pt.y, 1.0, 1e-9);
}

TEST(MemberBranchTest, GetIntersectionPoint_ParallelMembers_Throws) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 2.0, 0.0);
    auto n3 = makeNode(3, 0.0, 1.0);
    auto n4 = makeNode(4, 2.0, 1.0);  // parallel

    Member m1 = makeMember(1, n1, n2);
    Member m2 = makeMember(2, n3, n4);

    EXPECT_THROW(m1.getIntersectionPoint(m2), std::runtime_error);
}

// ---------------------------------------------------------------------------
// operator== / operator!=
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, OperatorEqual_SameIdAndNodes_ReturnsTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m1 = makeMember(10, n1, n2);
    Member m2 = makeMember(10, n1, n2);

    EXPECT_TRUE(m1 == m2);
    EXPECT_FALSE(m1 != m2);
}

TEST(MemberBranchTest, OperatorEqual_DifferentIds_ReturnsFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m1 = makeMember(10, n1, n2);
    Member m2 = makeMember(99, n1, n2);

    EXPECT_FALSE(m1 == m2);
    EXPECT_TRUE(m1 != m2);
}

// ---------------------------------------------------------------------------
// updateResults() – called via setAxialForce()
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, UpdateResults_PositiveForce_InTensionTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    // Default area=1e-4, yieldStrength=250e6  → threshold = 25000 N
    m.setAxialForce(5000.0);  // tension, below yield
    EXPECT_TRUE(m.isInTension());
    EXPECT_FALSE(m.hasYielded());
}

TEST(MemberBranchTest, UpdateResults_NegativeForce_InTensionFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    m.setAxialForce(-5000.0);  // compression, below yield
    EXPECT_FALSE(m.isInTension());
    EXPECT_FALSE(m.hasYielded());
}

TEST(MemberBranchTest, UpdateResults_ForceExceedsYield_YieldedTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    // Yield when |axialForce / area| > yieldStrength
    // area=1e-4, yieldStrength=250e6 → yield at |force| > 25000 N
    m.setAxialForce(30000.0);
    EXPECT_TRUE(m.isInTension());
    EXPECT_TRUE(m.hasYielded());
}

// Note: updateResults() computes utilizationRatio = axialStress/yieldStrength
// (no abs). For compression (negative force) the ratio is negative, so
// hasYielded() (> 1.0) returns false even when |force| > yield threshold.
// This test documents that actual behaviour.
TEST(MemberBranchTest, UpdateResults_CompressionLargeForce_YieldedFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    // |force| = 30000 N > threshold (25000 N) but compression → ratio < 0
    m.setAxialForce(-30000.0);
    EXPECT_FALSE(m.isInTension());
    EXPECT_FALSE(m.hasYielded());  // yielded flag only triggered for tension
}

// ---------------------------------------------------------------------------
// operator= – self-assignment branch (if this != &other → false)
// ---------------------------------------------------------------------------

/**
 * @brief Self-assignment must be a no-op and return *this (covers the
 *        false branch of `if (this != &other)` in Member::operator=).
 */
TEST(MemberBranchTest, CopyAssignment_SelfAssignment_ReturnsSelf) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 3.0, 4.0);
    Member m = makeMember(42, n1, n2);
    m.setAxialForce(100.0);

    // Self-assign – hits the false branch of `if (this != &other)`
    Member& ref = m;
    m = ref;  // NOLINT(clang-analyzer-cplusplus.SelfAssignment)

    // State unchanged after self-assignment
    EXPECT_EQ(m.getId(), 42);
    EXPECT_DOUBLE_EQ(m.getAxialForce(), 100.0);
}

/**
 * @brief Normal copy assignment copies all fields and is independent of source.
 */
TEST(MemberBranchTest, CopyAssignment_NormalCopy_CopiesAllFields) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 3.0, 4.0);
    Member src = makeMember(7, n1, n2);
    src.setAxialForce(250.0);

    auto n3 = makeNode(3, 0.0, 0.0);
    auto n4 = makeNode(4, 1.0, 0.0);
    Member dst = makeMember(99, n3, n4);

    dst = src;

    EXPECT_EQ(dst.getId(), 7);
    EXPECT_DOUBLE_EQ(dst.getAxialForce(), 250.0);
}

// ---------------------------------------------------------------------------
// getAngleDegrees() – never called before
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, GetAngleDegrees_HorizontalMember_ReturnsZero) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    EXPECT_DOUBLE_EQ(m.getAngleDegrees(), 0.0);
}

TEST(MemberBranchTest, GetAngleDegrees_DiagonalMember_Returns45) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 1.0);
    Member m = makeMember(1, n1, n2);

    EXPECT_NEAR(m.getAngleDegrees(), 45.0, 1e-9);
}

// ---------------------------------------------------------------------------
// isConnectedTo(const Node&) – both branches of ||
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, IsConnectedToNode_StartNode_ReturnsTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    Member m = makeMember(1, n1, n2);

    // Start node match → short-circuits, branch 0 taken true
    EXPECT_TRUE(m.isConnectedTo(*n1));
}

TEST(MemberBranchTest, IsConnectedToNode_EndNodeOnly_ReturnsTrue) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    auto n3 = makeNode(3, 2.0, 0.0);
    Member m = makeMember(1, n1, n2);

    // n3 is the end node — swap so n3 is n2's role
    // Use n2 as the query: start-node != n2, end-node == n2 → branch 1 taken
    EXPECT_TRUE(m.isConnectedTo(*n2));
}

TEST(MemberBranchTest, IsConnectedToNode_UnrelatedNode_ReturnsFalse) {
    auto n1 = makeNode(1, 0.0, 0.0);
    auto n2 = makeNode(2, 1.0, 0.0);
    auto n3 = makeNode(3, 5.0, 5.0);
    Member m = makeMember(1, n1, n2);

    EXPECT_FALSE(m.isConnectedTo(*n3));
}

// ---------------------------------------------------------------------------
// validateNodes() – null node throws std::invalid_argument
// ---------------------------------------------------------------------------

TEST(MemberBranchTest, Constructor_NullStartNode_ThrowsInvalidArgument) {
    auto n2 = makeNode(2, 1.0, 0.0);
    std::shared_ptr<Node> nullNode = nullptr;

    EXPECT_THROW(Member(1, nullNode, n2), std::invalid_argument);
}

TEST(MemberBranchTest, Constructor_NullEndNode_ThrowsInvalidArgument) {
    auto n1 = makeNode(1, 0.0, 0.0);
    std::shared_ptr<Node> nullNode = nullptr;

    EXPECT_THROW(Member(1, n1, nullNode), std::invalid_argument);
}
