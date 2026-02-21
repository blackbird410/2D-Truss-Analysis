/**
 * @file test_stiffness_matrix_bug.cpp
 * @brief Test to diagnose stiffness matrix sign error causing upward deformation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include <gtest/gtest.h>
#include "../../src/core/model/Member.hpp"
#include "../../src/core/model/Node.hpp"
#include <Eigen/Dense>
#include <cmath>

using namespace truss::core;

/**
 * @brief Test stiffness matrix for horizontal member
 * 
 * For a horizontal member (c=1, s=0):
 * K_global should be:
 * [ k   0  -k   0]
 * [ 0   0   0   0]
 * [-k   0   k   0]
 * [ 0   0   0   0]
 */
TEST(StiffnessMatrixBugTest, HorizontalMember_CheckTransformation) {
    // Create horizontal member
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(5.0, 0.0));
    
    MaterialProperties mat;
    mat.youngModulus = 200e9;  // 200 GPa
    
    SectionProperties sec;
    sec.area = 0.001;  // 1 cm²
    
    Member member(1, node1, node2, mat, sec);
    
    // Get global stiffness matrix
    MatrixXd K = member.getGlobalStiffnessMatrix();
    
    // Expected values
    Real EA = mat.youngModulus * sec.area;  // 200e6
    Real L = 5.0;
    Real k = EA / L;  // 40e6
    
    // Verify diagonal elements
    EXPECT_NEAR(K(0,0), k, k * 1e-10);
    EXPECT_NEAR(K(2,2), k, k * 1e-10);
    
    // Verify off-diagonal elements
    EXPECT_NEAR(K(0,2), -k, k * 1e-10);
    EXPECT_NEAR(K(2,0), -k, k * 1e-10);
    
    // Verify Y DOFs should be zero for horizontal member
    EXPECT_NEAR(K(1,1), 0.0, 1e-10);
    EXPECT_NEAR(K(3,3), 0.0, 1e-10);
}

/**
 * @brief Test stiffness matrix for vertical member
 * 
 * For a vertical member (c=0, s=1):
 * K_global should be:
 * [ 0   0   0   0]
 * [ 0   k   0  -k]
 * [ 0   0   0   0]
 * [ 0  -k   0   k]
 */
TEST(StiffnessMatrixBugTest, VerticalMember_CheckTransformation) {
    // Create vertical member
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(0.0, 3.0));
    
    MaterialProperties mat;
    mat.youngModulus = 200e9;
    
    SectionProperties sec;
    sec.area = 0.002;
    
    Member member(1, node1, node2, mat, sec);
    
    Real EA = mat.youngModulus * sec.area;
    Real L = 3.0;
    Real k = EA / L;
    
    MatrixXd K = member.getGlobalStiffnessMatrix();
    
    // X DOFs should be zero
    EXPECT_NEAR(K(0,0), 0.0, 1e-10);
    EXPECT_NEAR(K(2,2), 0.0, 1e-10);
    
    // Y DOF diagonal elements
    EXPECT_NEAR(K(1,1), k, k * 1e-10);
    EXPECT_NEAR(K(3,3), k, k * 1e-10);
    
    // Y DOF coupling
    EXPECT_NEAR(K(1,3), -k, k * 1e-10);
    EXPECT_NEAR(K(3,1), -k, k * 1e-10);
}

/**
 * @brief Test 45-degree diagonal member
 * 
 * For c = s = sqrt(2)/2:
 * K_global should be:
 * [ k/2   k/2  -k/2  -k/2]
 * [ k/2   k/2  -k/2  -k/2]
 * [-k/2  -k/2   k/2   k/2]
 * [-k/2  -k/2   k/2   k/2]
 */
TEST(StiffnessMatrixBugTest, DiagonalMember_CheckTransformation) {
    // Create 45-degree diagonal member
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 4.0));
    
    MaterialProperties mat;
    mat.youngModulus = 200e9;
    
    SectionProperties sec;
    sec.area = 0.002;
    
    Member member(1, node1, node2, mat, sec);
    
    Real EA = mat.youngModulus * sec.area;
    Real L = std::sqrt(32.0);  // sqrt(4^2 + 4^2)
    Real k = EA / L;
    
    MatrixXd K = member.getGlobalStiffnessMatrix();
    
    Real c = member.getUnitVector().x();
    Real s = member.getUnitVector().y();
    
    // Expected values
    Real c2 = c * c;
    Real s2 = s * s;
    Real cs = c * s;
    
    // Verify all elements match the analytical formula
    EXPECT_NEAR(K(0,0), k*c2, k * 1e-10);
    EXPECT_NEAR(K(0,1), k*cs, k * 1e-10);
    EXPECT_NEAR(K(1,1), k*s2, k * 1e-10);
    
    EXPECT_NEAR(K(0,2), -k*c2, k * 1e-10);
    EXPECT_NEAR(K(0,3), -k*cs, k * 1e-10);
    EXPECT_NEAR(K(1,3), -k*s2, k * 1e-10);
    
    EXPECT_NEAR(K(2,2), k*c2, k * 1e-10);
    EXPECT_NEAR(K(3,3), k*s2, k * 1e-10);
}

/**
 * @brief Test transformation matrix formula
 */
TEST(StiffnessMatrixBugTest, CheckTransformationMatrixFormula) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(3.0, 4.0));
    
    MaterialProperties mat;
    mat.youngModulus = 1.0;  // Simplified
    
    SectionProperties sec;
    sec.area = 1.0;  // Simplified
    
    Member member(1, node1, node2, mat, sec);
    
    Real c = member.getUnitVector().x();
    Real s = member.getUnitVector().y();
    
    EXPECT_NEAR(c, 0.6, 1e-10);
    EXPECT_NEAR(s, 0.8, 1e-10);
    
}
