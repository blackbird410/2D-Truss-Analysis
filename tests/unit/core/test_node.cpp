/**
 * @file test_node.cpp
 * @brief Google Test unit tests for Node class
 * @author Refactoring Agent (migrated from custom framework)
 * @version 3.0.0-dev
 * 
 * Migration Notes:
 * - Converted from custom TestFramework.hpp to Google Test
 * - Preserved all original test intent and coverage
 * - Maintained numerical tolerances (1e-10 for floating-point comparisons)
 * - Removed manual test registration (GTest auto-discovers tests)
 */

#include <gtest/gtest.h>
#include "../../src/core/Node.hpp"
#include "../../src/core/Types.hpp"

using namespace truss::core;

// Helper function for SupportType streaming (used by GTest for readable output)
std::ostream& operator<<(std::ostream& os, const SupportType& support) {
    switch (support) {
        case SupportType::Free: os << "Free"; break;
        case SupportType::PinnedX: os << "PinnedX"; break;
        case SupportType::PinnedY: os << "PinnedY"; break;
        case SupportType::Pinned: os << "Pinned"; break;
        case SupportType::RollerX: os << "RollerX"; break;
        case SupportType::RollerY: os << "RollerY"; break;
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

// GTest provides main() automatically via GTest::gtest_main
// No manual main() needed
