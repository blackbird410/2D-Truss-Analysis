#include <iostream>
#include "../TestFramework.hpp"
#include "../../src/core/Node.hpp"
#include "../../src/core/Types.hpp"

using namespace truss::core;

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

void test_node_creation() {
    Node node(1, Point2D(2.5, 3.7), SupportType::Free);
    
    ASSERT_EQ(node.getId(), 1);
    ASSERT_NEAR(node.getX(), 2.5, 1e-10);
    ASSERT_NEAR(node.getY(), 3.7, 1e-10);
    ASSERT_EQ(node.getSupportType(), SupportType::Free);
}

void test_node_support_types() {
    Node pinnedNode(1, Point2D(0.0, 0.0), SupportType::Pinned);
    Node rollerNode(2, Point2D(1.0, 0.0), SupportType::RollerX);
    Node freeNode(3, Point2D(2.0, 0.0), SupportType::Free);

    ASSERT_EQ(pinnedNode.getSupportType(), SupportType::Pinned);
    ASSERT_EQ(rollerNode.getSupportType(), SupportType::RollerX);
    ASSERT_EQ(freeNode.getSupportType(), SupportType::Free);
}

void test_node_force_application() {
    Node node(1, Point2D(0.0, 0.0), SupportType::Free);
    
    // Initially no force
    ASSERT_TRUE(node.hasAppliedForce() == false);
    
    // Apply force
    node.setAppliedForce(100.0, -200.0);
    
    ASSERT_TRUE(node.hasAppliedForce());
    Force2D appliedForce = node.getAppliedForce();
    ASSERT_NEAR(appliedForce.fx, 100.0, 1e-10);
    ASSERT_NEAR(appliedForce.fy, -200.0, 1e-10);
    
    // Clear force by setting to zero
    node.setAppliedForce(0.0, 0.0);
    ASSERT_TRUE(node.hasAppliedForce() == false);
}

void test_node_position_updates() {
    Node node(1, Point2D(3.0, 4.0), SupportType::Free);
    
    // Update position
    node.setPosition(10.0, -5.0);
    ASSERT_NEAR(node.getX(), 10.0, 1e-10);
    ASSERT_NEAR(node.getY(), -5.0, 1e-10);
}

void test_node_dof_management() {
    Node node(1, Point2D(0.0, 0.0), SupportType::Free);
    
    // Initially DOF should be 0 (default value)
    ASSERT_EQ(node.getDofX(), 0);
    ASSERT_EQ(node.getDofY(), 0);
    
    // Assign DOFs
    node.setDofX(5);
    node.setDofY(6);
    
    ASSERT_EQ(node.getDofX(), 5);
    ASSERT_EQ(node.getDofY(), 6);
}

void test_node_distance_calculation() {
    Node node1(1, Point2D(0.0, 0.0), SupportType::Free);
    Node node2(2, Point2D(3.0, 4.0), SupportType::Free);
    
    Real distance = node1.distanceTo(node2);
    ASSERT_NEAR(distance, 5.0, 1e-10);
    
    // Distance should be symmetric
    Real reverseDistance = node2.distanceTo(node1);
    ASSERT_NEAR(distance, reverseDistance, 1e-10);
}

int main() {
    using namespace truss::testing;
    
    TestFramework framework;
    
    framework.beginSuite("Node Class Tests");
    
    framework.runTest("Node creation and basic properties", test_node_creation);
    framework.runTest("Support type handling", test_node_support_types);
    framework.runTest("Force application and management", test_node_force_application);
    framework.runTest("Position updates", test_node_position_updates);
    framework.runTest("DOF management", test_node_dof_management);
    framework.runTest("Distance calculation", test_node_distance_calculation);
    
    framework.generateReport();
    
    return framework.allTestsPassed() ? 0 : 1;
}
