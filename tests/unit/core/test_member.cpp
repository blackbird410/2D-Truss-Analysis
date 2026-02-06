/**
 * @file test_member.cpp
 * @brief Google Test unit tests for Member class
 * @author Refactoring Agent (migrated from custom framework)
 * @version 3.0.0-dev
 * 
 * Migration Notes:
 * - Converted from custom TestFramework.hpp to Google Test
 * - Preserved all original test intent and coverage
 * - Maintained numerical tolerances (1e-9 for stiffness, 1e-3 for weight)
 * - Removed manual test registration (GTest auto-discovers tests)
 */

#include <gtest/gtest.h>
#include <memory>
#include "../../src/core/model/Member.hpp"
#include "../../src/core/model/Node.hpp"

using namespace truss::core;

// Test Suite: MemberTest
// Tests for Member class creation, properties, and calculations

TEST(MemberTest, CreationAndBasicProperties) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 3.0));
    MaterialProperties material;
    SectionProperties section;
    Member member(1, node1, node2, material, section);
    
    EXPECT_EQ(member.getId(), 1);
    EXPECT_EQ(member.getStartNode(), node1);
    EXPECT_EQ(member.getEndNode(), node2);
    EXPECT_DOUBLE_EQ(member.getLength(), 5.0);
}

TEST(MemberTest, StiffnessAndWeightCalculations) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 3.0));
    MaterialProperties material(200e9, 7850.0, 250e6, 400e6, "Steel"); // E, rho, fy, fu, name
    SectionProperties section(1e-4, 1e-8, 1e-4, "Default");            // A, I, As, designation
    Member member(1, node1, node2, material, section);
    
    // Expected stiffness = EA/L = 200e9 * 1e-4 / 5.0 = 4e6
    EXPECT_NEAR(member.getStiffness(), 4e6, 1e-9);
    // Expected weight = density * area * length = 7850 * 1e-4 * 5.0 = 3.925
    EXPECT_NEAR(member.getWeight(), 3.925, 1e-3);
}

TEST(MemberTest, GeometricChecks) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(10.0, 0.0));
    Member member(1, node1, node2);
    
    EXPECT_TRUE(member.isHorizontal());
    EXPECT_FALSE(member.isVertical());
}

TEST(MemberTest, AxialStiffnessCalculation) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(5.0, 0.0));
    MaterialProperties material(200e9, 7850.0, 250e6, 400e6, "Steel");
    SectionProperties section(0.001, 1e-8, 0.001, "Test");
    Member member(1, node1, node2, material, section);
    
    // Axial stiffness EA = E * A = 200e9 * 0.001 = 200e6
    EXPECT_NEAR(member.getAxialStiffness(), 200e6, 1e-3);
    
    // Member stiffness EA/L = 200e6 / 5.0 = 40e6
    EXPECT_NEAR(member.getStiffness(), 40e6, 1e-3);
}

TEST(MemberTest, NodeConnectivityQueries) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(3.0, 4.0));
    auto node3 = std::make_shared<Node>(3, Point2D(5.0, 0.0));
    Member member(1, node1, node2);
    
    // Test hasNode
    EXPECT_TRUE(member.hasNode(1));
    EXPECT_TRUE(member.hasNode(2));
    EXPECT_FALSE(member.hasNode(3));
    
    // Test isConnectedTo
    EXPECT_TRUE(member.isConnectedTo(1));
    EXPECT_TRUE(member.isConnectedTo(2));
    EXPECT_FALSE(member.isConnectedTo(3));
    
    // Test connectsNodes (both orders)
    EXPECT_TRUE(member.connectsNodes(1, 2));
    EXPECT_TRUE(member.connectsNodes(2, 1));  // Order shouldn't matter
    EXPECT_FALSE(member.connectsNodes(1, 3));
    EXPECT_FALSE(member.connectsNodes(2, 3));
    EXPECT_FALSE(member.connectsNodes(3, 1));
}

TEST(MemberTest, DirectionAndUnitVector) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(3.0, 4.0));
    Member member(1, node1, node2);
    
    // Direction vector
    Vector2d dir = member.getDirection();
    EXPECT_NEAR(dir.x(), 3.0, 1e-10);
    EXPECT_NEAR(dir.y(), 4.0, 1e-10);
    
    // Unit vector (normalized)
    Vector2d unit = member.getUnitVector();
    EXPECT_NEAR(unit.x(), 0.6, 1e-10);  // 3/5
    EXPECT_NEAR(unit.y(), 0.8, 1e-10);  // 4/5
    EXPECT_NEAR(unit.norm(), 1.0, 1e-10);
}

TEST(MemberTest, LocalStiffnessMatrix) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 0.0));
    MaterialProperties material(200e9, 7850.0, 250e6, 400e6, "Steel");
    SectionProperties section(0.01, 1e-6, 0.01, "Test");
    Member member(1, node1, node2, material, section);
    
    // EA/L = 200e9 * 0.01 / 4.0 = 500e6
    Real k = member.getStiffness();
    EXPECT_NEAR(k, 500e6, 1e-3);
    
    // Local stiffness matrix (4x4 for 2D truss element)
    MatrixXd K_local = member.getLocalStiffnessMatrix();
    
    ASSERT_EQ(K_local.rows(), 4);
    ASSERT_EQ(K_local.cols(), 4);
    
    // Expected local stiffness matrix for truss element:
    // [ k  0 -k  0]
    // [ 0  0  0  0]
    // [-k  0  k  0]
    // [ 0  0  0  0]
    
    EXPECT_NEAR(K_local(0, 0),  k, 1e-6);
    EXPECT_NEAR(K_local(0, 1),  0, 1e-10);
    EXPECT_NEAR(K_local(0, 2), -k, 1e-6);
    EXPECT_NEAR(K_local(0, 3),  0, 1e-10);
    
    EXPECT_NEAR(K_local(1, 0),  0, 1e-10);
    EXPECT_NEAR(K_local(1, 1),  0, 1e-10);
    EXPECT_NEAR(K_local(1, 2),  0, 1e-10);
    EXPECT_NEAR(K_local(1, 3),  0, 1e-10);
    
    EXPECT_NEAR(K_local(2, 0), -k, 1e-6);
    EXPECT_NEAR(K_local(2, 1),  0, 1e-10);
    EXPECT_NEAR(K_local(2, 2),  k, 1e-6);
    EXPECT_NEAR(K_local(2, 3),  0, 1e-10);
    
    EXPECT_NEAR(K_local(3, 0),  0, 1e-10);
    EXPECT_NEAR(K_local(3, 1),  0, 1e-10);
    EXPECT_NEAR(K_local(3, 2),  0, 1e-10);
    EXPECT_NEAR(K_local(3, 3),  0, 1e-10);
}

TEST(MemberTest, GlobalStiffnessMatrixHorizontal) {
    // Test global stiffness for horizontal member (no rotation)
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(2.0, 0.0));
    MaterialProperties material(100e9, 7850.0, 250e6, 400e6, "Steel");
    SectionProperties section(0.01, 1e-6, 0.01, "Test");
    Member member(1, node1, node2, material, section);
    
    Real k = member.getStiffness();  // EA/L = 100e9 * 0.01 / 2.0 = 500e6
    
    MatrixXd K_global = member.getGlobalStiffnessMatrix();
    
    ASSERT_EQ(K_global.rows(), 4);
    ASSERT_EQ(K_global.cols(), 4);
    
    // For horizontal member, global = local (no rotation needed)
    // Expected:
    // [ k  0 -k  0]
    // [ 0  0  0  0]
    // [-k  0  k  0]
    // [ 0  0  0  0]
    
    EXPECT_NEAR(K_global(0, 0),  k, 1e-6);
    EXPECT_NEAR(K_global(0, 2), -k, 1e-6);
    EXPECT_NEAR(K_global(2, 0), -k, 1e-6);
    EXPECT_NEAR(K_global(2, 2),  k, 1e-6);
    
    // Off-diagonal terms should be near zero for horizontal member
    EXPECT_NEAR(K_global(1, 1), 0, 1e-10);
    EXPECT_NEAR(K_global(3, 3), 0, 1e-10);
}

TEST(MemberTest, GlobalStiffnessMatrixVertical) {
    // Test global stiffness for vertical member (90-degree rotation)
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(0.0, 3.0));
    MaterialProperties material(100e9, 7850.0, 250e6, 400e6, "Steel");
    SectionProperties section(0.01, 1e-6, 0.01, "Test");
    Member member(1, node1, node2, material, section);
    
    Real k = member.getStiffness();  // EA/L = 100e9 * 0.01 / 3.0
    
    MatrixXd K_global = member.getGlobalStiffnessMatrix();
    
    ASSERT_EQ(K_global.rows(), 4);
    ASSERT_EQ(K_global.cols(), 4);
    
    // For vertical member, stiffness is in Y-direction
    // Main diagonal should have k in Y DOFs (indices 1 and 3)
    EXPECT_NEAR(K_global(1, 1),  k, 1e-6);
    EXPECT_NEAR(K_global(3, 3),  k, 1e-6);
    EXPECT_NEAR(K_global(1, 3), -k, 1e-6);
    EXPECT_NEAR(K_global(3, 1), -k, 1e-6);
    
    // X-direction should have minimal stiffness
    EXPECT_NEAR(K_global(0, 0), 0, 1e-10);
    EXPECT_NEAR(K_global(2, 2), 0, 1e-10);
}

TEST(MemberTest, GlobalStiffnessMatrixSymmetry) {
    // Test that global stiffness matrix is symmetric for any angle
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(3.0, 4.0));  // 3-4-5 triangle
    MaterialProperties material(200e9, 7850.0, 250e6, 400e6, "Steel");
    SectionProperties section(0.01, 1e-6, 0.01, "Test");
    Member member(1, node1, node2, material, section);
    
    MatrixXd K_global = member.getGlobalStiffnessMatrix();
    
    // Check symmetry: K(i,j) == K(j,i)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(K_global(i, j), K_global(j, i), 1e-10)
                << "Matrix not symmetric at (" << i << ", " << j << ")";
        }
    }
}

