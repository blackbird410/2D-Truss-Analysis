/**
 * @file test_node.cpp
 * @brief Google Test unit tests for Node class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Migration Notes:
 * - Converted from custom TestFramework.hpp to Google Test
 * - Preserved all original test intent and coverage
 * - Maintained numerical tolerances (1e-10 for floating-point comparisons)
 * - Removed manual test registration (GTest auto-discovers tests)
 */

#include "../../src/core/model/Node.hpp"
#include "../../src/core/model/Types.hpp"

#include <gtest/gtest.h>

using namespace truss::core;

// Helper function for SupportType streaming (used by GTest for readable output)
std::ostream& operator<<(std::ostream& os, const SupportType& support) {
    switch (support) {
        case SupportType::Free:
            os << "Free";
            break;
        case SupportType::Pinned:
            os << "Pinned";
            break;
        case SupportType::RollerX:
            os << "RollerX";
            break;
        case SupportType::RollerY:
            os << "RollerY";
            break;
    }
    return os;
}

// Test Suite: NodeTest
// Tests for Node class creation, properties, and behavior

TEST(NodeTest, CreationAndBasicProperties) {
    Node node(1, Point2D(2.5, 3.7), SupportType::Free);

    EXPECT_EQ(node.getId(), 1);
    EXPECT_NEAR(node.getX(), 2.5, 1e-10);
    EXPECT_NEAR(node.getY(), 3.7, 1e-10);
    EXPECT_EQ(node.getSupportType(), SupportType::Free);
}

TEST(NodeTest, SupportTypeHandling) {
    Node pinnedNode(1, Point2D(0.0, 0.0), SupportType::Pinned);
    Node rollerNode(2, Point2D(1.0, 0.0), SupportType::RollerX);
    Node freeNode(3, Point2D(2.0, 0.0), SupportType::Free);

    EXPECT_EQ(pinnedNode.getSupportType(), SupportType::Pinned);
    EXPECT_EQ(rollerNode.getSupportType(), SupportType::RollerX);
    EXPECT_EQ(freeNode.getSupportType(), SupportType::Free);
}

TEST(NodeTest, ForceApplicationAndManagement) {
    Node node(1, Point2D(0.0, 0.0), SupportType::Free);

    // Initially no force
    EXPECT_FALSE(node.hasAppliedForce());

    // Apply force
    node.setAppliedForce(100.0, -200.0);

    EXPECT_TRUE(node.hasAppliedForce());
    Force2D appliedForce = node.getAppliedForce();
    EXPECT_NEAR(appliedForce.fx, 100.0, 1e-10);
    EXPECT_NEAR(appliedForce.fy, -200.0, 1e-10);

    // Clear force by setting to zero
    node.setAppliedForce(0.0, 0.0);
    EXPECT_FALSE(node.hasAppliedForce());
}

TEST(NodeTest, PositionUpdates) {
    Node node(1, Point2D(3.0, 4.0), SupportType::Free);

    // Update position
    node.setPosition(10.0, -5.0);
    EXPECT_NEAR(node.getX(), 10.0, 1e-10);
    EXPECT_NEAR(node.getY(), -5.0, 1e-10);
}

TEST(NodeTest, DofManagement) {
    Node node(1, Point2D(0.0, 0.0), SupportType::Free);

    // Initially DOF should be 0 (default value)
    EXPECT_EQ(node.getDofX(), 0);
    EXPECT_EQ(node.getDofY(), 0);

    // Assign DOFs
    node.setDofX(5);
    node.setDofY(6);

    EXPECT_EQ(node.getDofX(), 5);
    EXPECT_EQ(node.getDofY(), 6);
}

TEST(NodeTest, DistanceCalculation) {
    Node node1(1, Point2D(0.0, 0.0), SupportType::Free);
    Node node2(2, Point2D(3.0, 4.0), SupportType::Free);

    Real distance = node1.distanceTo(node2);
    EXPECT_NEAR(distance, 5.0, 1e-10);

    // Distance should be symmetric
    Real reverseDistance = node2.distanceTo(node1);
    EXPECT_NEAR(distance, reverseDistance, 1e-10);
}

TEST(NodeTest, ConstraintChecking) {
    // Test isConstrained() for all support types
    Node freeNode(1, Point2D(0.0, 0.0), SupportType::Free);
    Node pinnedNode(2, Point2D(1.0, 0.0), SupportType::Pinned);
    Node rollerXNode(3, Point2D(2.0, 0.0), SupportType::RollerX);
    Node rollerYNode(4, Point2D(3.0, 0.0), SupportType::RollerY);

    EXPECT_FALSE(freeNode.isConstrained());
    EXPECT_TRUE(pinnedNode.isConstrained());
    EXPECT_TRUE(rollerXNode.isConstrained());
    EXPECT_TRUE(rollerYNode.isConstrained());
}

TEST(NodeTest, DegreesOfFreedom) {
    // Test getDegreesOfFreedom() for all support types
    Node freeNode(1, Point2D(0.0, 0.0), SupportType::Free);
    Node pinnedNode(2, Point2D(1.0, 0.0), SupportType::Pinned);
    Node rollerXNode(3, Point2D(2.0, 0.0), SupportType::RollerX);
    Node rollerYNode(4, Point2D(3.0, 0.0), SupportType::RollerY);

    EXPECT_EQ(freeNode.getDegreesOfFreedom(), 2);
    EXPECT_EQ(pinnedNode.getDegreesOfFreedom(), 0);
    EXPECT_EQ(rollerXNode.getDegreesOfFreedom(), 1);
    EXPECT_EQ(rollerYNode.getDegreesOfFreedom(), 1);
}

TEST(NodeTest, GlobalDOFRetrieval) {
    // Test getGlobalDOFs() for free node (both DOFs active)
    Node freeNode(1, Point2D(0.0, 0.0), SupportType::Free);
    freeNode.setDofX(10);
    freeNode.setDofY(11);

    std::vector<Index> freeDofs = freeNode.getGlobalDOFs();
    ASSERT_EQ(freeDofs.size(), 2);
    EXPECT_EQ(freeDofs[0], 10);
    EXPECT_EQ(freeDofs[1], 11);

    // Test pinned node (no DOFs)
    Node pinnedNode(2, Point2D(1.0, 0.0), SupportType::Pinned);
    pinnedNode.setDofX(20);
    pinnedNode.setDofY(21);

    std::vector<Index> pinnedDofs = pinnedNode.getGlobalDOFs();
    EXPECT_EQ(pinnedDofs.size(), 0);

    // Test RollerX node (only X DOF active, Y constrained)
    Node rollerXNode(3, Point2D(2.0, 0.0), SupportType::RollerX);
    rollerXNode.setDofX(30);
    rollerXNode.setDofY(31);

    std::vector<Index> rollerXDofs = rollerXNode.getGlobalDOFs();
    ASSERT_EQ(rollerXDofs.size(), 1);
    EXPECT_EQ(rollerXDofs[0], 30);  // Only X DOF

    // Test RollerY node (only Y DOF active, X constrained)
    Node rollerYNode(4, Point2D(3.0, 0.0), SupportType::RollerY);
    rollerYNode.setDofX(40);
    rollerYNode.setDofY(41);

    std::vector<Index> rollerYDofs = rollerYNode.getGlobalDOFs();
    ASSERT_EQ(rollerYDofs.size(), 1);
    EXPECT_EQ(rollerYDofs[0], 41);  // Only Y DOF
}

// Phase 7 Task 7.4: Domain Layer Enhancement - Edge Case Tests

TEST(NodeTest, ExtremeCoordinateValues) {
    // Very large coordinates
    Node largeNode(100, Point2D(1e6, 1e6), SupportType::Free);
    EXPECT_NEAR(largeNode.getX(), 1e6, 1e-6);
    EXPECT_NEAR(largeNode.getY(), 1e6, 1e-6);

    // Very small coordinates
    Node smallNode(101, Point2D(1e-8, 1e-8), SupportType::Free);
    EXPECT_NEAR(smallNode.getX(), 1e-8, 1e-14);
    EXPECT_NEAR(smallNode.getY(), 1e-8, 1e-14);
}

TEST(NodeTest, ZeroCoordinateEdgeCases) {
    // X=0, Y!=0
    Node node1(102, Point2D(0.0, 5.0), SupportType::Free);
    EXPECT_NEAR(node1.getX(), 0.0, 1e-10);
    EXPECT_NEAR(node1.getY(), 5.0, 1e-10);

    // X!=0, Y=0
    Node node2(103, Point2D(5.0, 0.0), SupportType::Free);
    EXPECT_NEAR(node2.getX(), 5.0, 1e-10);
    EXPECT_NEAR(node2.getY(), 0.0, 1e-10);

    // Both zero (origin)
    Node origin(104, Point2D(0.0, 0.0), SupportType::Pinned);
    EXPECT_NEAR(origin.getX(), 0.0, 1e-10);
    EXPECT_NEAR(origin.getY(), 0.0, 1e-10);
}

TEST(NodeTest, NegativeCoordinateHandling) {
    Node negNode(105, Point2D(-100.5, -200.3), SupportType::Free);
    EXPECT_NEAR(negNode.getX(), -100.5, 1e-10);
    EXPECT_NEAR(negNode.getY(), -200.3, 1e-10);
}
