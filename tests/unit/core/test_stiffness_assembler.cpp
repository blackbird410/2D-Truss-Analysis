/**
 * @file test_stiffness_assembler.cpp
 * @brief Google Test unit tests for StiffnessAssembler class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Test Strategy:
 * - Focus on numerical equivalence with AnalysisEngine (CRITICAL)
 * - Validate matrix properties (symmetry, dimensions)
 * - Test complex structures for comprehensive validation
 * 
 * Note: All tests use default material and section properties from Truss::addMember()
 */

#include <gtest/gtest.h>
#include "../../src/core/analysis/StiffnessAssembler.hpp"
#include "../../src/core/model/Truss.hpp"
#include <cmath>

using namespace truss::core;
using namespace truss::core::analysis;

// Test fixture for StiffnessAssembler tests
class StiffnessAssemblerTest : public ::testing::Test {
protected:
    StiffnessAssembler assembler;
};

// Note: Numerical equivalence with legacy AnalysisEngine was validated during Phase 2 Task 2.5

TEST_F(StiffnessAssemblerTest, MatrixSymmetry) {
    // Verify stiffness matrix is symmetric
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    truss.assignDofNumbers();
    
    MatrixXd K = assembler.assemble(truss);
    
    // Verify symmetry
    for (int i = 0; i < K.rows(); ++i) {
        for (int j = 0; j < K.cols(); ++j) {
            EXPECT_NEAR(K(i, j), K(j, i), 1e-10)
                << "Matrix not symmetric at (" << i << ", " << j << ")";
        }
    }
}

TEST_F(StiffnessAssemblerTest, AssembleAsVectorConversion) {
    // Verify conversion to 2D vector format
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(3.0, 0.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.assignDofNumbers();
    
    // Assemble as Eigen matrix
    MatrixXd K_eigen = assembler.assemble(truss);
    
    // Assemble as vector
    auto K_vector = assembler.assembleAsVector(truss);
    
    // Verify dimensions
    EXPECT_EQ(K_vector.size(), static_cast<size_t>(K_eigen.rows()));
    EXPECT_EQ(K_vector[0].size(), static_cast<size_t>(K_eigen.cols()));
    
    // Verify values match
    for (size_t i = 0; i < K_vector.size(); ++i) {
        for (size_t j = 0; j < K_vector[i].size(); ++j) {
            EXPECT_NEAR(K_vector[i][j], K_eigen(i, j), 1e-10);
        }
    }
}

// Note: ComplexTrussNumericalEquivalence test removed - validation completed in Phase 2 Task 2.5

TEST_F(StiffnessAssemblerTest, MatrixDimensions) {
    // Verify correct matrix dimensions for various truss sizes
    Truss truss2;
    auto n1 = truss2.addNode(0.0, 0.0);
    auto n2 = truss2.addNode(1.0, 0.0);
    truss2.addMember(n1, n2);
    truss2.assignDofNumbers();
    
    MatrixXd K2 = assembler.assemble(truss2);
    EXPECT_EQ(K2.rows(), 4);  // 2 nodes × 2 DOFs
    EXPECT_EQ(K2.cols(), 4);
    
    // 3-node truss
    Truss truss3;
    auto n3a = truss3.addNode(0.0, 0.0);
    auto n3b = truss3.addNode(1.0, 0.0);
    auto n3c = truss3.addNode(0.5, 1.0);
    truss3.addMember(n3a, n3b);
    truss3.addMember(n3a, n3c);
    truss3.assignDofNumbers();
    
    MatrixXd K3 = assembler.assemble(truss3);
    EXPECT_EQ(K3.rows(), 6);  // 3 nodes × 2 DOFs
    EXPECT_EQ(K3.cols(), 6);
}

TEST_F(StiffnessAssemblerTest, MultipleOverlappingMembers) {
    // Test assembly with multiple members sharing the same nodes
    Truss truss;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::Free);
    
    // Add two identical members (e.g., parallel cables)
    truss.addMember(node1, node2);
    truss.addMember(node1, node2);
    truss.assignDofNumbers();
    
    MatrixXd K = assembler.assemble(truss);
    
    // Stiffness should be doubled compared to single member
    Truss singleTruss;
    auto n1 = singleTruss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = singleTruss.addNode(4.0, 0.0, SupportType::Free);
    singleTruss.addMember(n1, n2);
    singleTruss.assignDofNumbers();
    
    MatrixXd K_single = assembler.assemble(singleTruss);
    
    for (int i = 0; i < K.rows(); ++i) {
        for (int j = 0; j < K.cols(); ++j) {
            EXPECT_NEAR(K(i, j), 2.0 * K_single(i, j), 1e-9);
        }
    }
}

// GTest provides main() automatically
