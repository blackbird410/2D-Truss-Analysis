/**
 * @file test_warren_truss_displacement_sign.cpp
 * @brief Simple test to verify displacement signs in Warren truss
 */

#include <gtest/gtest.h>
#include "../../src/core/model/Truss.hpp"
#include "../../src/core/model/Node.hpp"
#include "../../src/core/model/Member.hpp"
#include "../../src/core/analysis/AnalysisOrchestrator.hpp"
#include "../../src/core/analysis/SolverFactory.hpp"
#include "../../src/core/validation/TrussValidator.hpp"
#include <iostream>
#include <iomanip>

using namespace truss::core;
using namespace truss::core::analysis;
using namespace truss::core::validation;

TEST(WarrenTrussDisplacementTest, CheckSignConvention) {
    // Material and section from AnalysisOrchestratorTest fixture
    MaterialProperties material;
    material.youngModulus = 200e9;
    material.yieldStrength = 250e6;
    material.density = 7850.0;
    
    SectionProperties section;
    section.area = 0.001;
    
    Truss truss;
    
    // Create Warren truss with downward loads
    auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Pinned);
    auto node2 = std::make_shared<Node>(2, Point2D{4.0, 0.0}, SupportType::Free);
    auto node3 = std::make_shared<Node>(3, Point2D{4.0, 3.0}, SupportType::Free);
    auto node4 = std::make_shared<Node>(4, Point2D{8.0, 0.0}, SupportType::Free);
    auto node5 = std::make_shared<Node>(5, Point2D{12.0, 0.0}, SupportType::RollerX);  // FIXED: RollerX (Y constrained)
    
    // Apply DOWNWARD loads (NEGATIVE fy values)
    node2->setAppliedForce(Force2D{0.0, -5000.0});
    node3->setAppliedForce(Force2D{0.0, -10000.0});
    node4->setAppliedForce(Force2D{0.0, -5000.0});
    
    truss.addNode(node1);
    truss.addNode(node2);
    truss.addNode(node3);
    truss.addNode(node4);
    truss.addNode(node5);
    
    // Create members (statically determinate truss)
    auto member1 = std::make_shared<Member>(1, node1, node2, material, section);
    auto member2 = std::make_shared<Member>(2, node2, node4, material, section);
    auto member3 = std::make_shared<Member>(3, node4, node5, material, section);
    auto member4 = std::make_shared<Member>(4, node1, node3, material, section);
    auto member5 = std::make_shared<Member>(5, node3, node4, material, section);
    auto member6 = std::make_shared<Member>(6, node2, node3, material, section);
    auto member7 = std::make_shared<Member>(7, node3, node5, material, section);
    
    truss.addMember(member1);
    truss.addMember(member2);
    truss.addMember(member3);
    truss.addMember(member4);
    truss.addMember(member5);
    truss.addMember(member6);
    truss.addMember(member7);
    
    // Perform analysis
    analysis::AnalysisOptions options;
    options.computeReactions = true;
    
    auto solver = SolverFactory::createDirectSolver();
    auto validator = std::make_unique<TrussValidator>();
    AnalysisOrchestrator orchestrator(std::move(solver), std::make_unique<TrussValidator>(), options);
    
    auto results = orchestrator.analyze(truss);
    
    // Print diagnostic information
    std::cout << "\n========== DISPLACEMENT SIGN VERIFICATION ==========" << std::endl;
    std::cout << std::fixed << std::setprecision(6);
    
    std::cout << "\nApplied Loads (DOWNWARD = NEGATIVE fy):" << std::endl;
    std::cout << "  Node 2: fx = " << node2->getAppliedForce().fx 
              << ", fy = " << node2->getAppliedForce().fy << " N" << std::endl;
    std::cout << "  Node 3: fx = " << node3->getAppliedForce().fx 
              << ", fy = " << node3->getAppliedForce().fy << " N" << std::endl;
    std::cout << "  Node 4: fx = " << node4->getAppliedForce().fx 
              << ", fy = " << node4->getAppliedForce().fy << " N" << std::endl;
    
    std::cout << "\nDisplacements from Analysis Results:" << std::endl;
    std::cout << "  Node 1 (Pinned):  dx = " << node1->getDisplacement().x * 1000 
              << " mm, dy = " << node1->getDisplacement().y * 1000 << " mm" << std::endl;
    std::cout << "  Node 2 (Free):    dx = " << node2->getDisplacement().x * 1000 
              << " mm, dy = " << node2->getDisplacement().y * 1000 << " mm ← CRITICAL" << std::endl;
    std::cout << "  Node 3 (Free):    dx = " << node3->getDisplacement().x * 1000 
              << " mm, dy = " << node3->getDisplacement().y * 1000 << " mm ← CRITICAL" << std::endl;
    std::cout << "  Node 4 (Free):    dx = " << node4->getDisplacement().x * 1000 
              << " mm, dy = " << node4->getDisplacement().y * 1000 << " mm ← CRITICAL" << std::endl;
    std::cout << "  Node 5 (RollerX): dx = " << node5->getDisplacement().x * 1000 
              << " mm, dy = " << node5->getDisplacement().y * 1000 << " mm" << std::endl;
    
    std::cout << "\nSign Convention Check:" << std::endl;
    Real dy2 = node2->getDisplacement().y;
    Real dy3 = node3->getDisplacement().y;
    Real dy4 = node4->getDisplacement().y;
    
    if (dy2 < 0 && dy3 < 0 && dy4 < 0) {
        std::cout << "  ✓ CORRECT: All loaded nodes have NEGATIVE (downward) displacements" << std::endl;
    } else {
        std::cout << "  ✗ ERROR: Displacement sign convention is WRONG!" << std::endl;
        std::cout << "    Node 2: dy = " << dy2 << " (expected < 0)" << std::endl;
        std::cout << "    Node 3: dy = " << dy3 << " (expected < 0)" << std::endl;
        std::cout << "    Node 4: dy = " << dy4 << " (expected < 0)" << std::endl;
    }
    
    std::cout << "\nReactions (should sum to balance loads):" << std::endl;
    std::cout << "  Node 1 (Pinned):  rx = " << node1->getReaction().fx 
              << " N, ry = " << node1->getReaction().fy << " N" << std::endl;
    std::cout << "  Node 5 (RollerX): rx = " << node5->getReaction().fx 
              << " N, ry = " << node5->getReaction().fy << " N" << std::endl;
    
    Real sum_fy = node1->getReaction().fy + node5->getReaction().fy + 
                  node2->getAppliedForce().fy + node3->getAppliedForce().fy + node4->getAppliedForce().fy;
    
    std::cout << "\nEquilibrium Check:" << std::endl;
    std::cout << "  ΣFy = " << sum_fy << " N (should be ~0)" << std::endl;
    
    if (std::abs(sum_fy) < 1e-6) {
        std::cout << "  ✓ EQUILIBRIUM SATISFIED" << std::endl;
    } else {
        std::cout << "  ✗ EQUILIBRIUM VIOLATION" << std::endl;
    }
    
    std::cout << "====================================================\n" << std::endl;
    
    // Assertions
    EXPECT_TRUE(results.converged) << "Analysis must converge";
    EXPECT_LT(dy2, 0.0) << "Node 2 with downward load must have negative (downward) displacement";
    EXPECT_LT(dy3, 0.0) << "Node 3 with downward load must have negative (downward) displacement";
    EXPECT_LT(dy4, 0.0) << "Node 4 with downward load must have negative (downward) displacement";
    EXPECT_NEAR(sum_fy, 0.0, 1e-6) << "Equilibrium must be satisfied (ΣFy = 0)";
}
