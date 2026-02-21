/**
 * @file test_truss.cpp
 * @brief Google Test unit tests for Truss class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Migration Notes:
 * - Converted from custom TestFramework.hpp to Google Test
 * - Preserved all original test intent and coverage
 * - Maintained numerical tolerances (1e-10 for floating-point comparisons)
 * - Removed manual test registration (GTest auto-discovers tests)
 */

#include <gtest/gtest.h>
#include "../../src/core/model/Truss.hpp"
#include "../../src/core/validation/TrussValidator.hpp"

using namespace truss::core;

// Test Suite: TrussTest
// Tests for Truss class creation, node/member management, and validation

TEST(TrussTest, CreationAndBasicProperties) {
    Truss truss("Test Truss");
    
    EXPECT_EQ(truss.getName(), "Test Truss");
    EXPECT_EQ(truss.getNodeCount(), 0);
    EXPECT_EQ(truss.getMemberCount(), 0);
}

TEST(TrussTest, NodeManagement) {
    Truss truss;
    
    // Add nodes
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(Point2D(4.0, 0.0), SupportType::RollerX);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    EXPECT_EQ(truss.getNodeCount(), 3);
    EXPECT_EQ(node1->getId(), 1);
    EXPECT_EQ(node2->getId(), 2);
    EXPECT_EQ(node3->getId(), 3);
    
    // Test node retrieval
    auto retrievedNode = truss.getNode(2);
    EXPECT_EQ(retrievedNode, node2);
    
    // Test node removal
    bool removed = truss.removeNode(2);
    EXPECT_TRUE(removed);
    EXPECT_EQ(truss.getNodeCount(), 2);
    
    // Test removal of non-existent node
    removed = truss.removeNode(999);
    EXPECT_FALSE(removed);
}

TEST(TrussTest, MemberManagement) {
    Truss truss;
    
    // Add nodes first
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(4.0, 0.0);
    auto node3 = truss.addNode(2.0, 3.0);
    
    // Add members
    auto member1 = truss.addMember(node1, node2);
    auto member2 = truss.addMember(1, 3); // Using node IDs
    auto member3 = truss.addMember(node2, node3);
    
    EXPECT_EQ(truss.getMemberCount(), 3);
    EXPECT_EQ(member1->getId(), 1);
    EXPECT_EQ(member2->getId(), 2);
    EXPECT_EQ(member3->getId(), 3);
    
    // Test member retrieval
    auto retrievedMember = truss.getMember(2);
    EXPECT_EQ(retrievedMember, member2);
    
    // Test member removal
    bool removed = truss.removeMember(2);
    EXPECT_TRUE(removed);
    EXPECT_EQ(truss.getMemberCount(), 2);
}

TEST(TrussTest, ForceApplication) {
    Truss truss;
    
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(4.0, 0.0);
    
    // Apply forces
    truss.applyForce(1, Force2D(100.0, -200.0));
    truss.applyForce(2, 50.0, -100.0);
    
    EXPECT_TRUE(truss.hasAppliedForces());
    
    // Check forces were applied
    EXPECT_NEAR(node1->getAppliedForce().fx, 100.0, 1e-10);
    EXPECT_NEAR(node1->getAppliedForce().fy, -200.0, 1e-10);
    EXPECT_NEAR(node2->getAppliedForce().fx, 50.0, 1e-10);
    EXPECT_NEAR(node2->getAppliedForce().fy, -100.0, 1e-10);
    
    // Clear forces
    truss.clearForces();
    EXPECT_FALSE(truss.hasAppliedForces());
}

TEST(TrussTest, ValidationAndDeterminacy) {
    Truss truss;
    
    // Test validation using TrussValidator (replacement for deprecated methods)
    truss::core::validation::TrussValidator validator;
    
    // Empty truss should be invalid
    EXPECT_FALSE(validator.isValid(truss));
    
    // Add nodes and members to create a valid triangular truss
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    // Validate using TrussValidator
    auto result = validator.validate(truss);
    EXPECT_TRUE(result.isValid());
    EXPECT_FALSE(result.hasErrors());
}

TEST(TrussTest, StatisticsGeneration) {
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
    
    EXPECT_EQ(stats.totalNodes, 3);
    EXPECT_EQ(stats.totalMembers, 3);
    EXPECT_EQ(stats.appliedForces, 1);
    EXPECT_GT(stats.totalLength, 0.0);
}

TEST(TrussTest, GetMembersConnectedToNode) {
    Truss truss;
    
    // Create a truss with 4 nodes
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(4.0, 0.0);
    auto node3 = truss.addNode(4.0, 3.0);
    auto node4 = truss.addNode(0.0, 3.0);
    
    // Create members - node1 connects to 3 members, node3 to 2, node4 to 1
    auto member1 = truss.addMember(node1, node2);
    auto member2 = truss.addMember(node1, node3);
    auto member3 = truss.addMember(node1, node4);
    auto member4 = truss.addMember(node2, node3);
    
    // Test getMembersConnectedTo by NodeId
    auto members1 = truss.getMembersConnectedTo(1);
    EXPECT_EQ(members1.size(), 3);
    EXPECT_TRUE(std::find(members1.begin(), members1.end(), member1) != members1.end());
    EXPECT_TRUE(std::find(members1.begin(), members1.end(), member2) != members1.end());
    EXPECT_TRUE(std::find(members1.begin(), members1.end(), member3) != members1.end());
    
    // Test getMembersConnectedTo by NodePtr
    auto members3 = truss.getMembersConnectedTo(node3);
    EXPECT_EQ(members3.size(), 2);
    EXPECT_TRUE(std::find(members3.begin(), members3.end(), member2) != members3.end());
    EXPECT_TRUE(std::find(members3.begin(), members3.end(), member4) != members3.end());
    
    // Test getMembersAtNode (alias)
    auto members4 = truss.getMembersAtNode(4);
    EXPECT_EQ(members4.size(), 1);
    EXPECT_EQ(members4[0], member3);
    
    // Test non-existent node
    auto noMembers = truss.getMembersConnectedTo(999);
    EXPECT_EQ(noMembers.size(), 0);
    
    // Test with nullptr
    auto nullMembers = truss.getMembersConnectedTo(nullptr);
    EXPECT_EQ(nullMembers.size(), 0);
}

TEST(TrussTest, GetConstrainedNodes) {
    Truss truss;
    
    // Create nodes with various support types
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);      // constrained
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerX);     // constrained
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);        // free
    auto node4 = truss.addNode(6.0, 0.0, SupportType::RollerY);     // constrained
    auto node5 = truss.addNode(4.0, 3.0, SupportType::Free);        // free
    
    // Get constrained nodes
    auto constrainedNodes = truss.getConstrainedNodes();
    EXPECT_EQ(constrainedNodes.size(), 3);
    EXPECT_TRUE(std::find(constrainedNodes.begin(), constrainedNodes.end(), node1) != constrainedNodes.end());
    EXPECT_TRUE(std::find(constrainedNodes.begin(), constrainedNodes.end(), node2) != constrainedNodes.end());
    EXPECT_TRUE(std::find(constrainedNodes.begin(), constrainedNodes.end(), node4) != constrainedNodes.end());
    
    // Verify free nodes are not in the list
    EXPECT_TRUE(std::find(constrainedNodes.begin(), constrainedNodes.end(), node3) == constrainedNodes.end());
    EXPECT_TRUE(std::find(constrainedNodes.begin(), constrainedNodes.end(), node5) == constrainedNodes.end());
}

TEST(TrussTest, GetLoadedNodes) {
    Truss truss;
    
    // Create nodes
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(4.0, 0.0);
    auto node3 = truss.addNode(2.0, 3.0);
    auto node4 = truss.addNode(6.0, 0.0);
    
    // Apply forces to some nodes
    truss.applyForce(1, 100.0, -200.0);
    truss.applyForce(3, Force2D(0.0, -1000.0));
    // node2 and node4 have no applied forces
    
    // Get loaded nodes
    auto loadedNodes = truss.getLoadedNodes();
    EXPECT_EQ(loadedNodes.size(), 2);
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), node1) != loadedNodes.end());
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), node3) != loadedNodes.end());
    
    // Verify unloaded nodes are not in the list
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), node2) == loadedNodes.end());
    EXPECT_TRUE(std::find(loadedNodes.begin(), loadedNodes.end(), node4) == loadedNodes.end());
    
    // Clear forces and verify
    truss.clearForces();
    auto noLoadedNodes = truss.getLoadedNodes();
    EXPECT_EQ(noLoadedNodes.size(), 0);
}

TEST(TrussTest, GetFreeNodes) {
    Truss truss;
    
    // Create nodes with various support types
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);      // constrained
    auto node2 = truss.addNode(4.0, 0.0, SupportType::Free);        // free
    auto node3 = truss.addNode(2.0, 3.0, SupportType::RollerY);     // constrained
    auto node4 = truss.addNode(6.0, 0.0, SupportType::Free);        // free
    auto node5 = truss.addNode(4.0, 3.0, SupportType::RollerX);     // constrained
    
    // Get free nodes
    auto freeNodes = truss.getFreeNodes();
    EXPECT_EQ(freeNodes.size(), 2);
    EXPECT_TRUE(std::find(freeNodes.begin(), freeNodes.end(), node2) != freeNodes.end());
    EXPECT_TRUE(std::find(freeNodes.begin(), freeNodes.end(), node4) != freeNodes.end());
    
    // Verify constrained nodes are not in the list
    EXPECT_TRUE(std::find(freeNodes.begin(), freeNodes.end(), node1) == freeNodes.end());
    EXPECT_TRUE(std::find(freeNodes.begin(), freeNodes.end(), node3) == freeNodes.end());
    EXPECT_TRUE(std::find(freeNodes.begin(), freeNodes.end(), node5) == freeNodes.end());
}

TEST(TrussTest, GetNodesInRegion) {
    Truss truss;
    
    // Create a grid of nodes
    auto node1 = truss.addNode(0.0, 0.0);
    auto node2 = truss.addNode(2.0, 0.0);
    auto node3 = truss.addNode(4.0, 0.0);
    auto node4 = truss.addNode(0.0, 2.0);
    auto node5 = truss.addNode(2.0, 2.0);
    auto node6 = truss.addNode(4.0, 2.0);
    auto node7 = truss.addNode(0.0, 4.0);
    auto node8 = truss.addNode(2.0, 4.0);
    auto node9 = truss.addNode(4.0, 4.0);
    
    // Test region selection (1.0, 1.0) to (3.0, 3.0) - should include center node
    auto regionNodes = truss.getNodesInRegion(Point2D(1.0, 1.0), Point2D(3.0, 3.0));
    EXPECT_EQ(regionNodes.size(), 1);
    EXPECT_EQ(regionNodes[0], node5);
    
    // Test larger region (0.0, 0.0) to (2.0, 2.0) - should include 4 corner nodes
    auto cornerNodes = truss.getNodesInRegion(Point2D(0.0, 0.0), Point2D(2.0, 2.0));
    EXPECT_EQ(cornerNodes.size(), 4);
    EXPECT_TRUE(std::find(cornerNodes.begin(), cornerNodes.end(), node1) != cornerNodes.end());
    EXPECT_TRUE(std::find(cornerNodes.begin(), cornerNodes.end(), node2) != cornerNodes.end());
    EXPECT_TRUE(std::find(cornerNodes.begin(), cornerNodes.end(), node4) != cornerNodes.end());
    EXPECT_TRUE(std::find(cornerNodes.begin(), cornerNodes.end(), node5) != cornerNodes.end());
    
    // Test empty region
    auto emptyRegion = truss.getNodesInRegion(Point2D(10.0, 10.0), Point2D(20.0, 20.0));
    EXPECT_EQ(emptyRegion.size(), 0);
    
    // Test full region - all nodes
    auto allNodes = truss.getNodesInRegion(Point2D(0.0, 0.0), Point2D(4.0, 4.0));
    EXPECT_EQ(allNodes.size(), 9);
}

TEST(TrussTest, AggregateRootBehavior) {
    Truss truss;
    
    // Verify that Truss acts as aggregate root - owns all nodes and members
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    auto member1 = truss.addMember(node1, node2);
    auto member2 = truss.addMember(node1, node3);
    
    // Verify ownership - nodes and members are managed by truss
    EXPECT_EQ(truss.getNodeCount(), 3);
    EXPECT_EQ(truss.getMemberCount(), 2);
    
    // Verify retrieval through aggregate root
    EXPECT_EQ(truss.getNode(1), node1);
    EXPECT_EQ(truss.getNode(2), node2);
    EXPECT_EQ(truss.getMember(1), member1);
    
    // Verify state queries work correctly
    auto constrainedNodes = truss.getConstrainedNodes();
    EXPECT_EQ(constrainedNodes.size(), 2);  // node1 and node2
    
    auto freeNodes = truss.getFreeNodes();
    EXPECT_EQ(freeNodes.size(), 1);  // node3
    
    // Verify connectivity queries
    auto members1 = truss.getMembersAtNode(1);
    EXPECT_EQ(members1.size(), 2);  // both members connect to node1
    
    // Test aggregate root deletion - removing node should work
    EXPECT_TRUE(truss.removeNode(3));
    EXPECT_EQ(truss.getNodeCount(), 2);
    
    // Verify member collection reflects the structure
    const auto& allNodes = truss.getNodes();
    const auto& allMembers = truss.getMembers();
    EXPECT_EQ(allNodes.size(), 2);
    EXPECT_EQ(allMembers.size(), 2);
}

// Phase 7 Task 7.4: Domain Layer Enhancement - Edge Case Tests

TEST(TrussTest, EmptyTrussQueries) {
    Truss emptyTruss;

    // Query on empty truss should return empty results
    auto members = emptyTruss.getMembersAtNode(1);
    EXPECT_EQ(members.size(), 0);

    auto constrained = emptyTruss.getConstrainedNodes();
    EXPECT_EQ(constrained.size(), 0);

    auto loaded = emptyTruss.getLoadedNodes();
    EXPECT_EQ(loaded.size(), 0);
}

TEST(TrussTest, SingularNodeCollection) {
    Truss truss;
    auto node = truss.addNode(0.0, 0.0, SupportType::Pinned);

    // Query with single node
    auto members = truss.getMembersAtNode(node->getId());
    EXPECT_EQ(members.size(), 0);  // No members connected

    auto constrained = truss.getConstrainedNodes();
    EXPECT_EQ(constrained.size(), 1);  // Single pinned node
    EXPECT_EQ(constrained[0]->getId(), 1);
}

TEST(TrussTest, LargeCoordinateRangeTruss) {
    Truss truss;

    // Nodes spanning from micron to meter scale
    auto n1 = truss.addNode(1e-6, 1e-6, SupportType::Pinned);
    auto n2 = truss.addNode(Point2D(100.0, 100.0), SupportType::Free);
    auto n3 = truss.addNode(1000.0, 1000.0, SupportType::Free);

    // Verify coordinate retrieval accuracy
    auto nodes = truss.getNodes();
    EXPECT_NEAR(nodes[0]->getX(), 1e-6, 1e-12);
    EXPECT_NEAR(nodes[2]->getX(), 1000.0, 1e-10);
}

// GTest provides main() automatically via GTest::gtest_main
// No manual main() needed
