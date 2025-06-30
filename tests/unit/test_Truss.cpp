#include "../TestFramework.hpp"
#include "../../src/core/Truss.hpp"

using namespace truss::core;
using namespace truss::testing;

void test_truss_creation() {
    Truss truss("Test Truss");
    
    ASSERT_EQ(truss.getName(), "Test Truss");
    ASSERT_EQ(truss.getNodeCount(), 0);
    ASSERT_EQ(truss.getMemberCount(), 0);
}

void test_node_management() {
    Truss truss;
    
    // Add nodes
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(Point2D(4.0, 0.0), SupportType::RollerX);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    ASSERT_EQ(truss.getNodeCount(), 3);
    ASSERT_EQ(node1->getId(), 1);
    ASSERT_EQ(node2->getId(), 2);
    ASSERT_EQ(node3->getId(), 3);
    
    // Test node retrieval
    auto retrievedNode = truss.getNode(2);
    ASSERT_EQ(retrievedNode, node2);
    
    // Test node removal
    bool removed = truss.removeNode(2);
    ASSERT_TRUE(removed);
    ASSERT_EQ(truss.getNodeCount(), 2);
    
    // Test removal of non-existent node
    removed = truss.removeNode(999);
    ASSERT_FALSE(removed);
}

void test_member_management() {
    Truss truss;
    
    // Add nodes first
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(4.0, 0.0);
    auto node3 = truss.addNode(2.0, 3.0);
    
    // Add members
    auto member1 = truss.addMember(node1, node2);
    auto member2 = truss.addMember(1, 3); // Using node IDs
    auto member3 = truss.addMember(node2, node3);
    
    ASSERT_EQ(truss.getMemberCount(), 3);
    ASSERT_EQ(member1->getId(), 1);
    ASSERT_EQ(member2->getId(), 2);
    ASSERT_EQ(member3->getId(), 3);
    
    // Test member retrieval
    auto retrievedMember = truss.getMember(2);
    ASSERT_EQ(retrievedMember, member2);
    
    // Test member removal
    bool removed = truss.removeMember(2);
    ASSERT_TRUE(removed);
    ASSERT_EQ(truss.getMemberCount(), 2);
}

void test_force_application() {
    Truss truss;
    
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(4.0, 0.0);
    
    // Apply forces
    truss.applyForce(1, Force2D(100.0, -200.0));
    truss.applyForce(2, 50.0, -100.0);
    
    ASSERT_TRUE(truss.hasAppliedForces());
    
    // Check forces were applied
    ASSERT_NEAR(node1->getAppliedForce().fx, 100.0, 1e-10);
    ASSERT_NEAR(node1->getAppliedForce().fy, -200.0, 1e-10);
    ASSERT_NEAR(node2->getAppliedForce().fx, 50.0, 1e-10);
    ASSERT_NEAR(node2->getAppliedForce().fy, -100.0, 1e-10);
    
    // Clear forces
    truss.clearForces();
    ASSERT_FALSE(truss.hasAppliedForces());
}

void test_truss_validation() {
    Truss truss;
    
    // Empty truss should be invalid
    ASSERT_FALSE(truss.isValid());
    
    // Add nodes and members to create a valid triangular truss
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    ASSERT_TRUE(truss.isValid());
    ASSERT_TRUE(truss.isStaticallyDeterminate());
}

void test_truss_statistics() {
    Truss truss("Statistics Test");
    
    // Create a simple truss
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    truss.applyForce(3, 0.0, -1000.0);
    
    auto stats = truss.getStatistics();
    
    ASSERT_EQ(stats.totalNodes, 3);
    ASSERT_EQ(stats.totalMembers, 3);
    ASSERT_EQ(stats.appliedForces, 1);
    ASSERT_GT(stats.totalLength, 0.0);
}

int main() {
    TestFramework framework;
    
    framework.beginSuite("Truss Class Tests");
    
    framework.runTest("Truss creation and basic properties", test_truss_creation);
    framework.runTest("Node management (add/remove/retrieve)", test_node_management);
    framework.runTest("Member management (add/remove/retrieve)", test_member_management);
    framework.runTest("Force application and management", test_force_application);
    framework.runTest("Truss validation and determinacy", test_truss_validation);
    framework.runTest("Truss statistics generation", test_truss_statistics);
    
    framework.generateReport();
    
    return framework.allTestsPassed() ? 0 : 1;
}
