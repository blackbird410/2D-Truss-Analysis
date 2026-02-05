/**
 * @file test_truss.cpp
 * @brief Google Test unit tests for Truss class
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
#include "../../src/core/Truss.hpp"

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
    
    // Empty truss should be invalid
    EXPECT_FALSE(truss.isValid());
    
    // Add nodes and members to create a valid triangular truss
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    EXPECT_TRUE(truss.isValid());
    EXPECT_TRUE(truss.isStaticallyDeterminate());
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

// GTest provides main() automatically via GTest::gtest_main
// No manual main() needed
