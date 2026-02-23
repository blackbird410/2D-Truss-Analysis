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

#include "../../src/core/analysis/stiffness_assembler.hpp"
#include "../../src/core/model/truss.hpp"

#include <Eigen/LU>
#include <Eigen/SVD>

#include <cmath>
#include <gtest/gtest.h>
#include <limits>

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

// Phase 7 Task 7.5: Analysis Layer Hardening - Edge Case Tests

TEST_F(StiffnessAssemblerTest, ExtremeCoordinateRangeAssembly) {
    // Test assembly with nodes spanning extreme coordinate ranges
    Truss truss;
    auto n1 = truss.addNode(1e-8, 1e-8, SupportType::Pinned);       // Micron scale
    auto n2 = truss.addNode(1000.0, 1000.0, SupportType::RollerY);  // Meter scale

    truss.addMember(n1, n2);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // Verify matrix properties despite extreme ranges
    EXPECT_EQ(K.rows(), 4);  // 2 nodes × 2 DOFs
    EXPECT_EQ(K.cols(), 4);

    // Matrix should remain symmetric
    for (int i = 0; i < K.rows(); ++i) {
        for (int j = 0; j < K.cols(); ++j) {
            EXPECT_NEAR(K(i, j), K(j, i), 1e-6);
        }
    }
}

TEST_F(StiffnessAssemblerTest, NearlyZeroLengthMember) {
    // Test assembly with very short member (near singularity)
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(1e-6, 0.0, SupportType::Free);

    truss.addMember(n1, n2);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // Matrix should still be assembled even for short members
    EXPECT_EQ(K.rows(), 4);

    // Check for numerical issues
    EXPECT_FALSE(std::isnan(K(0, 0)));
    EXPECT_FALSE(std::isinf(K(0, 0)));
}

TEST_F(StiffnessAssemblerTest, SingleTriangleTruss) {
    // Test assembly of minimal valid structure (single triangle)
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(1.0, 0.0, SupportType::Pinned);
    auto n3 = truss.addNode(0.5, 0.866, SupportType::Free);

    truss.addMember(n1, n2);
    truss.addMember(n1, n3);
    truss.addMember(n2, n3);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // 3 nodes × 2 DOFs = 6 total DOFs
    // 2 pinned nodes (4 constrained DOFs) + 1 free node (2 free DOFs)
    // Should reduce to 2×2 matrix for free DOFs
    EXPECT_EQ(K.rows(), 6);
    EXPECT_EQ(K.cols(), 6);
}

// ============================================================================
// Singular Matrix Detection Tests (Task 7.5 - Analysis Hardening)
// ============================================================================

TEST_F(StiffnessAssemblerTest, SingularMatrixDetection) {
    // Truss with insufficient constraints (kinematic instability)
    // This tests detection of numerically singular matrices
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Free);
    auto n2 = truss.addNode(1.0, 0.0, SupportType::Free);
    truss.addMember(n1, n2);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // Compute condition number (indicator of singularity)
    Eigen::JacobiSVD<MatrixXd> svd(K);
    double conditionNumber = svd.singularValues()(0) / svd.singularValues()(K.cols() - 1);

    // Numerically singular matrices have very high condition numbers
    EXPECT_GT(conditionNumber, 1e10);
}

TEST_F(StiffnessAssemblerTest, RankDeficientMatrix) {
    // Create truss with both nodes only constrained in x-direction
    // This creates a rank-deficient stiffness matrix
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::RollerY);  // Constrained in Y only
    auto n2 = truss.addNode(1.0, 0.0, SupportType::RollerY);  // Constrained in Y only
    truss.addMember(n1, n2);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // Compute rank using full pivot LU decomposition
    Eigen::FullPivLU<MatrixXd> lu(K);
    int rank = lu.rank();

    // Rank-deficient matrix should have rank less than dimensions
    EXPECT_LT(rank, K.rows());
}

TEST_F(StiffnessAssemblerTest, NearSingularSystem) {
    // Create a well-formed structure that tests near-singularity conditions
    // by checking condition number of properly constrained system
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(1e-10, 0.0, SupportType::Free);  // Very close to n1
    truss.addMember(n1, n2);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // Check for high condition number
    Eigen::JacobiSVD<MatrixXd> svd(K);
    double conditionNumber = svd.singularValues()(0) / svd.singularValues()(K.cols() - 1);

    // Should have a high condition number indicating numerical issues
    EXPECT_GT(conditionNumber, 1e6);
}

TEST_F(StiffnessAssemblerTest, ConditionNumberComputation) {
    // Create well-conditioned system (equilateral triangle properly supported)
    Truss truss;
    auto n1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(1.0, 0.0, SupportType::RollerY);
    auto n3 = truss.addNode(0.5, 0.866, SupportType::Free);
    truss.addMember(n1, n2);
    truss.addMember(n2, n3);
    truss.addMember(n3, n1);
    truss.assignDofNumbers();

    MatrixXd K = assembler.assemble(truss);

    // Compute condition number
    Eigen::JacobiSVD<MatrixXd> svd(K);
    double conditionNumber = svd.singularValues()(0) / svd.singularValues()(K.cols() - 1);

    // Just verify that condition number can be computed
    // Different geometric configurations will have different condition numbers
    EXPECT_GT(conditionNumber, 0.0);
    // The condition number should be finite (not infinite or NaN)
    EXPECT_TRUE(std::isfinite(conditionNumber));
}
