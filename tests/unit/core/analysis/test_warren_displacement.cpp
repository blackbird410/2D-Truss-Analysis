/**
 * @file test_warren_displacement.cpp
 * @brief Simple test to verify displacement signs in Warren truss.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "../../src/core/analysis/analysis_orchestrator.hpp"
#include "../../src/core/analysis/solver_factory.hpp"
#include "../../src/core/model/member.hpp"
#include "../../src/core/model/node.hpp"
#include "../../src/core/model/truss.hpp"
#include "../../src/core/validation/truss_validator.hpp"

#include <gtest/gtest.h>

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
    auto node5 = std::make_shared<Node>(
        5, Point2D{12.0, 0.0}, SupportType::RollerX);  // FIXED: RollerX (Y constrained)

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
    AnalysisOrchestrator orchestrator(
        std::move(solver), std::make_unique<TrussValidator>(), options);

    auto results = orchestrator.analyze(truss);

    Real dy2 = node2->getDisplacement().y;
    Real dy3 = node3->getDisplacement().y;
    Real dy4 = node4->getDisplacement().y;

    Real sum_fy = node1->getReaction().fy + node5->getReaction().fy + node2->getAppliedForce().fy +
                  node3->getAppliedForce().fy + node4->getAppliedForce().fy;

    // Assertions
    EXPECT_TRUE(results.converged) << "Analysis must converge";
    EXPECT_LT(dy2, 0.0) << "Node 2 with downward load must have negative (downward) displacement";
    EXPECT_LT(dy3, 0.0) << "Node 3 with downward load must have negative (downward) displacement";
    EXPECT_LT(dy4, 0.0) << "Node 4 with downward load must have negative (downward) displacement";
    EXPECT_NEAR(sum_fy, 0.0, 1e-6) << "Equilibrium must be satisfied (ΣFy = 0)";
}
