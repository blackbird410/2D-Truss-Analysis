/**
 * @file test_displacement_sign_check.cpp
 * @brief Test to verify displacement signs with downward loads
 * 
 * Purpose: Confirm that downward loads produce negative (downward) displacements
 * in the analysis results, ruling out computation errors.
 */

#include <gtest/gtest.h>
#include "../../src/core/model/Truss.hpp"
#include "../../src/core/model/Node.hpp"
#include "../../src/core/model/Member.hpp"
#include "../../src/core/analysis/AnalysisOrchestrator.hpp"
#include "../../src/core/analysis/BoundaryConditionHandler.hpp"
#include "../../src/core/analysis/SolverFactory.hpp"
#include "../../src/core/validation/TrussValidator.hpp"
#include <iostream>
#include <iomanip>

using namespace truss::core;
using namespace truss::core::analysis;
using namespace truss::core::validation;

/**
 * Test: Simple triangle truss with downward load
 * 
 * Structure:
 *   Node 1 (0,0) - Pinned support
 *   Node 2 (2,0) - RollerX support (Y constrained, X free)
 *   Node 3 (1,1) - Free node with downward load
 *   Members: 1-2, 1-3, 2-3 (stable triangle)
 * 
 * Load: -1000 N at node 3 (downward)
 * 
 * Expected: Node 3 displacement dy < 0 (negative = downward)
 */
TEST(DisplacementSignTest, DownwardLoad_ProducesNegativeDisplacement) {
    // Create truss
    Truss truss;
    
    // Create nodes - forming stable triangle
    auto node1 = std::make_shared<Node>(1, 0.0, 0.0);
    auto node2 = std::make_shared<Node>(2, 2.0, 0.0);
    auto node3 = std::make_shared<Node>(3, 1.0, 1.0);  // Top node
    
    // Set supports - two supports for stability
    node1->setSupportType(SupportType::Pinned);  // Fixed support
    node2->setSupportType(SupportType::RollerX); // Y constrained
    node3->setSupportType(SupportType::Free);    // Free node
    
    // Apply downward load at node 3
    node3->setAppliedForce(0.0, -1000.0);  // DOWNWARD = NEGATIVE fy
    
    truss.addNode(node1);
    truss.addNode(node2);
    truss.addNode(node3);
    
    // Create members (stable triangle)
    MaterialProperties mat;
    mat.youngModulus = 200e9;  // Steel
    mat.yieldStrength = 250e6;
    mat.density = 7850.0;
    
    SectionProperties sec;
    sec.area = 0.001;  // 1000 mm²
    
    auto member1 = std::make_shared<Member>(1, node1, node2, mat, sec);
    auto member2 = std::make_shared<Member>(2, node1, node3, mat, sec);
    auto member3 = std::make_shared<Member>(3, node2, node3, mat, sec);
    
    truss.addMember(member1);
    truss.addMember(member2);
    truss.addMember(member3);
    
    // Perform analysis
    auto solver = SolverFactory::createDirectSolver();
    auto validator = std::make_unique<TrussValidator>();
    AnalysisOrchestrator orchestrator(std::move(solver), std::move(validator));
    
    auto results = orchestrator.analyze(truss);
    
    // Print diagnostics
    std::cout << "\n=== DISPLACEMENT SIGN TEST ===" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    std::cout << "\nApplied Load:" << std::endl;
    std::cout << "  Node 3: fy = " << node3->getAppliedForce().fy << " N (DOWNWARD)" << std::endl;
    
    std::cout << "\nDisplacements from analysis:" << std::endl;
    std::cout << "  Node 1: dx = " << node1->getDisplacement().x 
              << ", dy = " << node1->getDisplacement().y << std::endl;
    std::cout << "  Node 2: dx = " << node2->getDisplacement().x 
              << ", dy = " << node2->getDisplacement().y << std::endl;
    std::cout << "  Node 3: dx = " << node3->getDisplacement().x 
              << ", dy = " << node3->getDisplacement().y << " ← CRITICAL VALUE" << std::endl;
    
    // CRITICAL TEST: Node 3 dy must be NEGATIVE (downward)
    Real dy_node3 = node3->getDisplacement().y;
    
    std::cout << "\n=== VERIFICATION ===" << std::endl;
    if (dy_node3 < 0) {
        std::cout << "✓ CORRECT: dy = " << dy_node3 << " (negative = downward)" << std::endl;
    } else {
        std::cout << "✗ ERROR: dy = " << dy_node3 << " (should be negative!)" << std::endl;
    }
    
    // Verify equilibrium
    std::cout << "\nReactions:" << std::endl;
    std::cout << "  Node 1: rx = " << node1->getReaction().fx 
              << ", ry = " << node1->getReaction().fy << std::endl;
    std::cout << "  Node 2: rx = " << node2->getReaction().fx 
              << ", ry = " << node2->getReaction().fy << std::endl;
    
    Real sum_fy = node1->getReaction().fy + node2->getReaction().fy + node3->getAppliedForce().fy;
    std::cout << "\nEquilibrium Check (ΣFy = 0):" << std::endl;
    std::cout << "  ΣFy = " << sum_fy << std::endl;
    
    if (std::abs(sum_fy) < 1e-6) {
        std::cout << "✓ EQUILIBRIUM SATISFIED" << std::endl;
    } else {
        std::cout << "✗ EQUILIBRIUM VIOLATION" << std::endl;
    }
    
    std::cout << "===========================\n" << std::endl;
    
    // ASSERTIONS
    EXPECT_TRUE(results.converged) << "Analysis should converge";
    EXPECT_LT(dy_node3, 0.0) << "Downward load MUST produce negative displacement (dy < 0)";
    EXPECT_NEAR(sum_fy, 0.0, 1e-6) << "Equilibrium must be satisfied";
}

/**
 * Test: Stable truss with downward load
 * 
 * Structure:
 *   Node 1 (0,0) - Pinned support (bottom left)
 *   Node 2 (2,0) - RollerX support (bottom right)
 *   Node 3 (1,2) - Free node (top) with downward load
 *   Members: 1-2, 1-3, 2-3 (stable triangle)
 * 
 * Load: -1000 N at node 3 (downward)
 * 
 * Expected: 
 *   - Node 3 dy < 0 (downward displacement)
 */
TEST(DisplacementSignTest, VerticalColumn_DownwardLoad) {
    Truss truss;
    
    auto node1 = std::make_shared<Node>(1, 0.0, 0.0);
    auto node2 = std::make_shared<Node>(2, 2.0, 0.0);
    auto node3 = std::make_shared<Node>(3, 1.0, 2.0);
    
    node1->setSupportType(SupportType::Pinned);
    node2->setSupportType(SupportType::RollerX);  // Y constrained
    node3->setSupportType(SupportType::Free);
    node3->setAppliedForce(0.0, -1000.0);  // DOWNWARD
    
    truss.addNode(node1);
    truss.addNode(node2);
    truss.addNode(node3);
    
    MaterialProperties mat;
    mat.youngModulus = 200e9;
    mat.yieldStrength = 250e6;
    mat.density = 7850.0;
    
    SectionProperties sec;
    sec.area = 0.001;
    
    auto member1 = std::make_shared<Member>(1, node1, node2, mat, sec);
    auto member2 = std::make_shared<Member>(2, node1, node3, mat, sec);
    auto member3 = std::make_shared<Member>(3, node2, node3, mat, sec);
    truss.addMember(member1);
    truss.addMember(member2);
    truss.addMember(member3);
    
    auto solver = SolverFactory::createDirectSolver();
    auto validator = std::make_unique<TrussValidator>();
    AnalysisOrchestrator orchestrator(std::move(solver), std::move(validator));
    
    auto results = orchestrator.analyze(truss);
    
    Real dy_node3 = node3->getDisplacement().y;
    
    std::cout << "\n=== STABLE TRUSS TEST ===" << std::endl;
    std::cout << "Applied load: fy = " << node3->getAppliedForce().fy << " N" << std::endl;
    std::cout << "Node 3 displacement: dy = " << dy_node3 << std::endl;
    
    if (dy_node3 < 0) {
        std::cout << "✓ CORRECT: Downward load produces negative displacement" << std::endl;
    } else {
        std::cout << "✗ ERROR: Expected negative displacement" << std::endl;
    }
    std::cout << "===========================\n" << std::endl;
    
    EXPECT_LT(dy_node3, 0.0) << "Downward load must produce negative (downward) displacement";
}
