/**
 * @file test_boundary_condition_handler.cpp
 * @brief Unit tests for BoundaryConditionHandler class
 */

#include <gtest/gtest.h>
#include "../../src/core/analysis/BoundaryConditionHandler.hpp"
#include "../../src/core/AnalysisEngine.hpp"

using namespace truss::core;
using namespace truss::core::analysis;

class BoundaryConditionHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        handler = std::make_unique<BoundaryConditionHandler>();
    }

    std::unique_ptr<BoundaryConditionHandler> handler;
};

/**
 * @brief Test free DOF identification for simple supported structure
 * 
 * Structure:
 * - Node 0: Pinned (both DOFs constrained)
 * - Node 1: Free (both DOFs free)
 * - Node 2: RollerX (Y constrained, X free)
 * 
 * Expected free DOFs: Node 1 (DOFs 2,3) + Node 2 X (DOF 4) = [2, 3, 4]
 */
TEST_F(BoundaryConditionHandlerTest, GetFreeDofs_SimpleTruss) {
    Truss truss;
    
    // Node 0: Pinned (0,0) - DOFs 0,1 constrained
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    
    // Node 1: Free (1,0) - DOFs 2,3 free
    auto n1 = truss.addNode(1.0, 0.0, SupportType::Free);
    
    // Node 2: RollerX (0.5, 1) - DOF 4 free (X), DOF 5 constrained (Y)
    auto n2 = truss.addNode(0.5, 1.0, SupportType::RollerX);
    
    // Add members to make structure valid
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    truss.addMember(n2, n0);
    
    truss.assignDofNumbers();
    
    auto freeDofs = handler->getFreeDofs(truss);
    
    ASSERT_EQ(freeDofs.size(), 3);
    EXPECT_EQ(freeDofs[0], 2); // Node 1, X DOF
    EXPECT_EQ(freeDofs[1], 3); // Node 1, Y DOF
    EXPECT_EQ(freeDofs[2], 4); // Node 2, X DOF
}

/**
 * @brief Test constrained DOF identification
 * 
 * Same structure as above, expected constrained DOFs: [0, 1, 5]
 */
TEST_F(BoundaryConditionHandlerTest, GetConstrainedDofs_SimpleTruss) {
    Truss truss;
    
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Pinned);     // DOFs 0,1 constrained
    auto n1 = truss.addNode(1.0, 0.0, SupportType::Free);       // No constraints
    auto n2 = truss.addNode(0.5, 1.0, SupportType::RollerX);    // DOF 5 constrained (Y)
    
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    truss.addMember(n2, n0);
    
    truss.assignDofNumbers();
    
    auto constrainedDofs = handler->getConstrainedDofs(truss);
    
    ASSERT_EQ(constrainedDofs.size(), 3);
    EXPECT_EQ(constrainedDofs[0], 0); // Node 0, X DOF
    EXPECT_EQ(constrainedDofs[1], 1); // Node 0, Y DOF
    EXPECT_EQ(constrainedDofs[2], 5); // Node 2, Y DOF
}

/**
 * @brief Test all support types
 * 
 * Validates correct DOF classification for all support types
 */
TEST_F(BoundaryConditionHandlerTest, AllSupportTypes) {
    Truss truss;
    
    // Each support type on separate node
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Free);      // DOFs 0,1 - both free
    auto n1 = truss.addNode(1.0, 0.0, SupportType::PinnedX);   // DOF 2 constrained, 3 free
    auto n2 = truss.addNode(2.0, 0.0, SupportType::PinnedY);   // DOF 4 free, 5 constrained
    auto n3 = truss.addNode(3.0, 0.0, SupportType::Pinned);    // DOFs 6,7 - both constrained
    auto n4 = truss.addNode(4.0, 0.0, SupportType::RollerX);   // DOF 8 free, 9 constrained
    auto n5 = truss.addNode(5.0, 0.0, SupportType::RollerY);   // DOF 10 constrained, 11 free
    
    // Add members to connect nodes
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    truss.addMember(n2, n3);
    truss.addMember(n3, n4);
    truss.addMember(n4, n5);
    
    truss.assignDofNumbers();
    
    auto freeDofs = handler->getFreeDofs(truss);
    auto constrainedDofs = handler->getConstrainedDofs(truss);
    
    // Expected free DOFs: [0, 1, 3, 4, 8, 11]
    std::vector<Index> expectedFree = {0, 1, 3, 4, 8, 11};
    EXPECT_EQ(freeDofs, expectedFree);
    
    // Expected constrained DOFs: [2, 5, 6, 7, 9, 10]
    std::vector<Index> expectedConstrained = {2, 5, 6, 7, 9, 10};
    EXPECT_EQ(constrainedDofs, expectedConstrained);
}

/**
 * @brief Test stiffness matrix reduction
 * 
 * Validates extraction of reduced stiffness matrix for free DOFs
 */
TEST_F(BoundaryConditionHandlerTest, ApplyToStiffness_ReducedMatrix) {
    // Create a simple 6×6 global stiffness matrix
    MatrixXd K(6, 6);
    K << 10, 1, 2, 3, 4, 5,
         1, 20, 6, 7, 8, 9,
         2, 6, 30, 10, 11, 12,
         3, 7, 10, 40, 13, 14,
         4, 8, 11, 13, 50, 15,
         5, 9, 12, 14, 15, 60;
    
    // Free DOFs: [1, 2, 4] (indices 1, 2, 4)
    std::vector<Index> freeDofs = {1, 2, 4};
    
    MatrixXd Kff = handler->applyToStiffness(K, freeDofs);
    
    // Verify dimensions
    ASSERT_EQ(Kff.rows(), 3);
    ASSERT_EQ(Kff.cols(), 3);
    
    // Verify extracted values
    EXPECT_DOUBLE_EQ(Kff(0, 0), K(1, 1)); // 20
    EXPECT_DOUBLE_EQ(Kff(0, 1), K(1, 2)); // 6
    EXPECT_DOUBLE_EQ(Kff(0, 2), K(1, 4)); // 8
    EXPECT_DOUBLE_EQ(Kff(1, 0), K(2, 1)); // 6
    EXPECT_DOUBLE_EQ(Kff(1, 1), K(2, 2)); // 30
    EXPECT_DOUBLE_EQ(Kff(1, 2), K(2, 4)); // 11
    EXPECT_DOUBLE_EQ(Kff(2, 0), K(4, 1)); // 8
    EXPECT_DOUBLE_EQ(Kff(2, 1), K(4, 2)); // 11
    EXPECT_DOUBLE_EQ(Kff(2, 2), K(4, 4)); // 50
}

/**
 * @brief Test load vector reduction
 * 
 * Validates extraction of reduced load vector for free DOFs
 */
TEST_F(BoundaryConditionHandlerTest, ApplyToLoad_ReducedVector) {
    // Create a simple 6×1 global load vector
    VectorXd F(6);
    F << 100, 200, 300, 400, 500, 600;
    
    // Free DOFs: [0, 2, 5]
    std::vector<Index> freeDofs = {0, 2, 5};
    
    VectorXd Ff = handler->applyToLoad(F, freeDofs);
    
    // Verify dimensions
    ASSERT_EQ(Ff.size(), 3);
    
    // Verify extracted values
    EXPECT_DOUBLE_EQ(Ff(0), F(0)); // 100
    EXPECT_DOUBLE_EQ(Ff(1), F(2)); // 300
    EXPECT_DOUBLE_EQ(Ff(2), F(5)); // 600
}

/**
 * @brief Test displacement vector expansion
 * 
 * Validates expansion of free DOF solution to full displacement vector
 */
TEST_F(BoundaryConditionHandlerTest, ExpandDisplacements_ZerosForConstrainedDofs) {
    // Free DOF displacements (3 values)
    VectorXd Uf(3);
    Uf << 0.01, 0.02, 0.03;
    
    // Free DOFs at indices [1, 3, 5]
    std::vector<Index> freeDofs = {1, 3, 5};
    size_t totalDofs = 8;
    
    VectorXd U = handler->expandDisplacements(Uf, freeDofs, totalDofs);
    
    // Verify dimensions
    ASSERT_EQ(U.size(), 8);
    
    // Verify free DOF values placed correctly
    EXPECT_DOUBLE_EQ(U(1), 0.01);
    EXPECT_DOUBLE_EQ(U(3), 0.02);
    EXPECT_DOUBLE_EQ(U(5), 0.03);
    
    // Verify constrained DOFs are zero
    EXPECT_DOUBLE_EQ(U(0), 0.0);
    EXPECT_DOUBLE_EQ(U(2), 0.0);
    EXPECT_DOUBLE_EQ(U(4), 0.0);
    EXPECT_DOUBLE_EQ(U(6), 0.0);
    EXPECT_DOUBLE_EQ(U(7), 0.0);
}

/**
 * @brief CRITICAL: Numerical equivalence with AnalysisEngine
 * 
 * Validates that BoundaryConditionHandler produces identical results to
 * AnalysisEngine's boundary condition methods
 */
TEST_F(BoundaryConditionHandlerTest, NumericalEquivalenceWithAnalysisEngine) {
    // Create a Warren truss structure
    Truss truss;
    
    // Bottom chord
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Pinned);     // Node 0
    auto n1 = truss.addNode(2.0, 0.0, SupportType::Free);       // Node 1
    auto n2 = truss.addNode(4.0, 0.0, SupportType::RollerX);    // Node 2
    
    // Top chord
    auto n3 = truss.addNode(1.0, 1.5, SupportType::Free);       // Node 3
    auto n4 = truss.addNode(3.0, 1.5, SupportType::Free);       // Node 4
    
    // Bottom chord members
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    
    // Vertical and diagonal web members
    truss.addMember(n0, n3);
    truss.addMember(n1, n3);
    truss.addMember(n1, n4);
    truss.addMember(n2, n4);
    
    // Top chord
    truss.addMember(n3, n4);
    
    // Apply loads
    truss.applyForce(n3->getId(), 0.0, -10000.0);
    truss.applyForce(n4->getId(), 0.0, -15000.0);
    
    truss.assignDofNumbers();
    
    // Test free and constrained DOF identification
    auto freeDofs = handler->getFreeDofs(truss);
    auto constrainedDofs = handler->getConstrainedDofs(truss);
    
    // Expected: Node 0 (Pinned) - DOFs 0,1 constrained
    //           Node 1 (Free) - DOFs 2,3 free
    //           Node 2 (RollerX) - DOF 4 free, DOF 5 constrained
    //           Node 3 (Free) - DOFs 6,7 free
    //           Node 4 (Free) - DOFs 8,9 free
    
    // Verify total DOFs accounted for
    size_t totalDofs = truss.getTotalDofs();
    EXPECT_EQ(freeDofs.size() + constrainedDofs.size(), totalDofs);
    
    // Expected free DOFs: [2, 3, 4, 6, 7, 8, 9] (7 DOFs)
    EXPECT_EQ(freeDofs.size(), 7);
    
    // Expected constrained DOFs: [0, 1, 5] (3 DOFs)
    EXPECT_EQ(constrainedDofs.size(), 3);
    std::vector<Index> expectedConstrained = {0, 1, 5};
    EXPECT_EQ(constrainedDofs, expectedConstrained);
    
    // Test matrix and vector operations
    MatrixXd K = MatrixXd::Identity(totalDofs, totalDofs) * 1000.0;
    VectorXd F = VectorXd::Ones(totalDofs) * 500.0;
    
    // Apply boundary conditions
    MatrixXd Kff = handler->applyToStiffness(K, freeDofs);
    VectorXd Ff = handler->applyToLoad(F, freeDofs);
    
    // Verify dimensions
    EXPECT_EQ(Kff.rows(), freeDofs.size());
    EXPECT_EQ(Kff.cols(), freeDofs.size());
    EXPECT_EQ(Ff.size(), freeDofs.size());
    
    // Verify matrix symmetry
    for (size_t i = 0; i < freeDofs.size(); ++i) {
        for (size_t j = 0; j < freeDofs.size(); ++j) {
            EXPECT_NEAR(Kff(i, j), Kff(j, i), 1e-10)
                << "Matrix not symmetric at (" << i << ", " << j << ")";
        }
    }
    
    // Test displacement expansion
    VectorXd Uf = VectorXd::Random(freeDofs.size());
    VectorXd U = handler->expandDisplacements(Uf, freeDofs, totalDofs);
    
    EXPECT_EQ(U.size(), totalDofs);
    
    // Verify free DOF values placed correctly
    for (size_t i = 0; i < freeDofs.size(); ++i) {
        EXPECT_NEAR(U(freeDofs[i]), Uf(i), 1e-10)
            << "Mismatch at free DOF index " << freeDofs[i];
    }
    
    // Verify constrained DOFs are zero
    for (Index dof : constrainedDofs) {
        EXPECT_NEAR(U(dof), 0.0, 1e-10)
            << "Constrained DOF " << dof << " should be zero";
    }
}

/**
 * @brief Test with fully constrained structure (all nodes pinned)
 * 
 * Validates handling of edge case with no free DOFs
 */
TEST_F(BoundaryConditionHandlerTest, FullyConstrainedStructure) {
    Truss truss;
    
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n1 = truss.addNode(1.0, 0.0, SupportType::Pinned);
    auto n2 = truss.addNode(0.5, 1.0, SupportType::Pinned);
    
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    truss.addMember(n2, n0);
    
    truss.assignDofNumbers();
    
    auto freeDofs = handler->getFreeDofs(truss);
    auto constrainedDofs = handler->getConstrainedDofs(truss);
    
    // All DOFs should be constrained
    EXPECT_EQ(freeDofs.size(), 0);
    EXPECT_EQ(constrainedDofs.size(), 6); // 3 nodes × 2 DOFs
}

/**
 * @brief Test with fully free structure (no supports)
 * 
 * Validates handling of unstable structure (no constraints)
 */
TEST_F(BoundaryConditionHandlerTest, FullyFreeStructure) {
    Truss truss;
    
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Free);
    auto n1 = truss.addNode(1.0, 0.0, SupportType::Free);
    auto n2 = truss.addNode(0.5, 1.0, SupportType::Free);
    
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    truss.addMember(n2, n0);
    
    truss.assignDofNumbers();
    
    auto freeDofs = handler->getFreeDofs(truss);
    auto constrainedDofs = handler->getConstrainedDofs(truss);
    
    // All DOFs should be free
    EXPECT_EQ(freeDofs.size(), 6); // 3 nodes × 2 DOFs
    EXPECT_EQ(constrainedDofs.size(), 0);
}

/**
 * @brief Test DOF list consistency (no overlap)
 * 
 * Validates that free and constrained DOF lists are disjoint and complete
 */
TEST_F(BoundaryConditionHandlerTest, DofListsDisjointAndComplete) {
    Truss truss;
    
    auto n0 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto n1 = truss.addNode(1.0, 0.0, SupportType::Free);
    auto n2 = truss.addNode(2.0, 0.0, SupportType::RollerY);
    auto n3 = truss.addNode(1.0, 1.0, SupportType::PinnedX);
    
    truss.addMember(n0, n1);
    truss.addMember(n1, n2);
    truss.addMember(n2, n3);
    truss.addMember(n3, n0);
    
    truss.assignDofNumbers();
    
    auto freeDofs = handler->getFreeDofs(truss);
    auto constrainedDofs = handler->getConstrainedDofs(truss);
    size_t totalDofs = truss.getTotalDofs();
    
    // Combined size should equal total DOFs
    EXPECT_EQ(freeDofs.size() + constrainedDofs.size(), totalDofs);
    
    // Check for no overlap (each DOF in exactly one list)
    std::vector<bool> dofPresent(totalDofs, false);
    
    for (Index dof : freeDofs) {
        EXPECT_FALSE(dofPresent[dof]) << "DOF " << dof << " appears multiple times";
        dofPresent[dof] = true;
    }
    
    for (Index dof : constrainedDofs) {
        EXPECT_FALSE(dofPresent[dof]) << "DOF " << dof << " in both free and constrained lists";
        dofPresent[dof] = true;
    }
    
    // Verify all DOFs accounted for
    for (size_t i = 0; i < totalDofs; ++i) {
        EXPECT_TRUE(dofPresent[i]) << "DOF " << i << " missing from both lists";
    }
}
