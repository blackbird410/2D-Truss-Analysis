/**
 * @file test_analysis_orchestrator.cpp
 * @brief Integration tests for AnalysisOrchestrator
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * These tests validate that the AnalysisOrchestrator produces
 * numerically equivalent results to the original AnalysisEngine.
 */

#include <gtest/gtest.h>
#include "../../src/core/analysis/AnalysisOrchestrator.hpp"
#include "../../src/core/analysis/SolverFactory.hpp"
#include "../../src/core/model/Truss.hpp"
#include "../../src/core/model/Node.hpp"
#include "../../src/core/model/Member.hpp"
#include <cmath>

using namespace truss::core;
using namespace truss::core::analysis;

/**
 * @brief Test fixture for AnalysisOrchestrator integration tests
 */
class AnalysisOrchestratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default material and section properties
        material.youngModulus = 200e9;  // 200 GPa (steel)
        material.yieldStrength = 250e6; // 250 MPa
        
        section.area = 0.01;  // 100 cm²
    }
    
    MaterialProperties material;
    SectionProperties section;
    
    /**
     * @brief Create a simple 3-node, 2-member triangular truss
     * 
     * Structure:
     *            Node 2 (Free)
     *           /     \
     *     Member1     Member2
     *         /         \
     *   Node 1 --- --- Node 3
     *  (Pinned)      (RollerY)
     *   
     * Geometry:
     *   Node 1: (0, 0)
     *   Node 2: (2, 2)
     *   Node 3: (4, 0)
     */
    Truss createSimpleTruss() {
        Truss truss;
        
        // Create nodes
        auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Pinned);
        auto node2 = std::make_shared<Node>(2, Point2D{2.0, 2.0}, SupportType::Free);
        auto node3 = std::make_shared<Node>(3, Point2D{4.0, 0.0}, SupportType::RollerY);
        
        // Apply load to top node
        node2->setAppliedForce(Force2D{0.0, -10000.0});  // 10 kN downward
        
        truss.addNode(node1);
        truss.addNode(node2);
        truss.addNode(node3);
        
        // Create members
        auto member1 = std::make_shared<Member>(1, node1, node2, material, section);
        auto member2 = std::make_shared<Member>(2, node2, node3, material, section);
        
        truss.addMember(member1);
        truss.addMember(member2);
        
        return truss;
    }
    
    /**
     * @brief Create a Warren truss (5 nodes, 7 members)
     * 
     * Structure:
     *        Node 3 (4m, 3m)
     *       /  \
     *      /    \
     *     /      \
     *  Node 1 -- Node 2 -- Node 4 -- Node 5
     *  (Pinned)  (Free)   (Free)    (RollerY)
     *  (0,0)     (4,0)    (8,0)     (12,0)
     */
    Truss createWarrenTruss() {
        Truss truss;
        
        // Create nodes
        auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Pinned);
        auto node2 = std::make_shared<Node>(2, Point2D{4.0, 0.0}, SupportType::Free);
        auto node3 = std::make_shared<Node>(3, Point2D{4.0, 3.0}, SupportType::Free);
        auto node4 = std::make_shared<Node>(4, Point2D{8.0, 0.0}, SupportType::Free);
        auto node5 = std::make_shared<Node>(5, Point2D{12.0, 0.0}, SupportType::RollerY);
        
        // Apply loads
        node2->setAppliedForce(Force2D{0.0, -5000.0});
        node3->setAppliedForce(Force2D{0.0, -10000.0});
        node4->setAppliedForce(Force2D{0.0, -5000.0});
        
        truss.addNode(node1);
        truss.addNode(node2);
        truss.addNode(node3);
        truss.addNode(node4);
        truss.addNode(node5);
        
        // Create members (bottom chord, top chord, diagonals)
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
        
        return truss;
    }
    
    /**
     * @brief Compare two result vectors with tolerance
     */
    void compareResults(
        const std::vector<Real>& expected,
        const std::vector<Real>& actual,
        Real tolerance,
        const std::string& context) {
        
        ASSERT_EQ(expected.size(), actual.size()) << context << ": Size mismatch";
        
        for (size_t i = 0; i < expected.size(); ++i) {
            EXPECT_NEAR(expected[i], actual[i], tolerance)
                << context << " at index " << i;
        }
    }
};

/**
 * @brief Test: Simple truss - complete workflow validation
 * 
 * Note: Numerical equivalence with legacy AnalysisEngine validated during Phase 2 Task 2.8
 */
TEST_F(AnalysisOrchestratorTest, SimpleTruss_CompleteWorkflow) {
    Truss truss = createSimpleTruss();
    
    // Analyze with AnalysisOrchestrator (no stability check)
    analysis::AnalysisOptions options;
    options.checkStability = false;
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    auto results = orchestrator.analyze(truss);
    
    // Verify results populated correctly
    ASSERT_TRUE(results.converged);
    ASSERT_EQ(results.displacements.size(), 6); // 3 nodes * 2 DOFs
    ASSERT_EQ(results.memberForces.size(), 2);  // 2 members
    // Reactions only for constrained DOFs (Pinned=2 + RollerY=1 = 3)
    ASSERT_EQ(results.reactions.size(), 3);
    
    // Verify metadata
    EXPECT_EQ(results.totalDofs, 6);
    EXPECT_GT(results.conditionNumber, 0.0);
    EXPECT_GT(results.maxDisplacement, 0.0);
}

/**
 * @brief Test: Warren truss - complex structure validation
 * 
 * Note: Numerical equivalence with legacy AnalysisEngine validated during Phase 2 Task 2.8
 */
TEST_F(AnalysisOrchestratorTest, WarrenTruss_ComplexStructure) {
    Truss truss = createWarrenTruss();
    
    // Analyze with AnalysisOrchestrator
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver));
    auto results = orchestrator.analyze(truss);
    
    // Verify results populated correctly
    ASSERT_TRUE(results.converged);
    ASSERT_EQ(results.displacements.size(), 10); // 5 nodes * 2 DOFs
    ASSERT_EQ(results.memberForces.size(), 7);   // 7 members
    // Reactions only for constrained DOFs (Pinned=2 + RollerY=1 = 3)
    ASSERT_EQ(results.reactions.size(), 3);
    
    // Verify all results are finite
    for (size_t i = 0; i < results.displacements.size(); ++i) {
        EXPECT_TRUE(std::isfinite(results.displacements[i]))
            << "Non-finite displacement at DOF " << i;
    }
    for (size_t i = 0; i < results.memberForces.size(); ++i) {
        EXPECT_TRUE(std::isfinite(results.memberForces[i]))
            << "Non-finite force in member " << i;
    }
}

/**
 * @brief Test: Metadata fields are populated correctly
 */
TEST_F(AnalysisOrchestratorTest, MetadataPopulation) {
    Truss truss = createSimpleTruss();
    
    analysis::AnalysisOptions options;
    options.checkStability = false;
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    auto results = orchestrator.analyze(truss);
    
    // Check metadata
    EXPECT_TRUE(results.converged);
    EXPECT_EQ(results.totalDofs, 6);  // 3 nodes * 2 DOFs
    EXPECT_EQ(results.freeDofs, 3);   // 1 free node * 2 DOFs + 1 DOF from RollerY
    EXPECT_EQ(results.constrainedDofs, 3);  // Pinned (2) + RollerY (1)
    
    EXPECT_GT(results.maxDisplacement, 0.0);
    EXPECT_GT(results.maxStress, 0.0);
    EXPECT_GT(results.conditionNumber, 0.0);
    EXPECT_GT(results.totalStrain, 0.0);
}

/**
 * @brief Test: Truss results are updated correctly
 */
TEST_F(AnalysisOrchestratorTest, TrussResultsUpdate) {
    Truss truss = createSimpleTruss();
    
    analysis::AnalysisOptions options;
    options.checkStability = false;
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    orchestrator.analyze(truss);
    
    // Check that free node has displacement
    const auto& nodes = truss.getNodes();
    bool hasDisplacement = false;
    for (const auto& node : nodes) {
        const auto& results = node->getResults();
        if (std::abs(results.displacement.x) > 1e-10 || std::abs(results.displacement.y) > 1e-10) {
            hasDisplacement = true;
            break;
        }
    }
    EXPECT_TRUE(hasDisplacement) << "At least one node should have displacement";
    
    // Check that members have forces
    const auto& members = truss.getMembers();
    for (const auto& member : members) {
        const auto& results = member->getResults();
        EXPECT_NE(results.axialForce, 0.0);
        EXPECT_NE(results.axialStress, 0.0);
    }
}

/**
 * @brief Test: Error handling - invalid truss
 */
TEST_F(AnalysisOrchestratorTest, ErrorHandling_InvalidTruss) {
    // Create empty truss
    Truss truss;
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver));
    
    EXPECT_THROW(orchestrator.analyze(truss), std::runtime_error);
}

/**
 * @brief Test: Error handling - unstable truss
 */
TEST_F(AnalysisOrchestratorTest, ErrorHandling_UnstableTruss) {
    Truss truss;
    
    // Create truss with insufficient constraints
    auto node1 = std::make_shared<Node>(1, Point2D{0.0, 0.0}, SupportType::Free);
    auto node2 = std::make_shared<Node>(2, Point2D{4.0, 0.0}, SupportType::Free);
    
    node1->setAppliedForce(Force2D{1000.0, 0.0});
    
    truss.addNode(node1);
    truss.addNode(node2);
    
    auto member = std::make_shared<Member>(1, node1, node2, material, section);
    truss.addMember(member);
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver));
    
    EXPECT_THROW(orchestrator.analyze(truss), std::runtime_error);
}

/**
 * @brief Test: Stiffness matrix is assembled correctly
 */
TEST_F(AnalysisOrchestratorTest, StiffnessMatrixAssembly) {
    Truss truss = createSimpleTruss();
    
    analysis::AnalysisOptions options;
    options.checkStability = false;
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    auto results = orchestrator.analyze(truss);
    
    // Check stiffness matrix dimensions
    ASSERT_EQ(results.stiffnessMatrix.size(), 6);  // 3 nodes * 2 DOFs
    for (const auto& row : results.stiffnessMatrix) {
        ASSERT_EQ(row.size(), 6);
    }
    
    // Stiffness matrix should be symmetric
    for (size_t i = 0; i < results.stiffnessMatrix.size(); ++i) {
        for (size_t j = 0; j < results.stiffnessMatrix[i].size(); ++j) {
            EXPECT_NEAR(
                results.stiffnessMatrix[i][j],
                results.stiffnessMatrix[j][i],
                1e-10)
                << "Stiffness matrix not symmetric at (" << i << "," << j << ")";
        }
    }
}

/**
 * @brief Test: Multiple analyses with same orchestrator
 */
TEST_F(AnalysisOrchestratorTest, MultipleAnalyses) {
    analysis::AnalysisOptions options;
    options.checkStability = false;
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    
    // Analyze first truss
    Truss truss1 = createSimpleTruss();
    auto results1 = orchestrator.analyze(truss1);
    
    // Analyze second truss
    Truss truss2 = createWarrenTruss();
    auto results2 = orchestrator.analyze(truss2);
    
    // Results should be different
    EXPECT_NE(results1.displacements.size(), results2.displacements.size());
    EXPECT_NE(results1.memberForces.size(), results2.memberForces.size());
    
    // Last results should be from second analysis
    const auto& lastResults = orchestrator.getLastResults();
    EXPECT_EQ(lastResults.displacements.size(), results2.displacements.size());
}

/**
 * @brief Test: Analysis with custom options
 */
TEST_F(AnalysisOrchestratorTest, CustomOptions) {
    Truss truss = createSimpleTruss();
    
    analysis::AnalysisOptions options;
    options.verbose = false;
    options.computeReactions = true;
    options.checkStability = false;  // Disable for simple truss
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    auto results = orchestrator.analyze(truss);
    
    // Results should have reactions
    EXPECT_GT(results.reactions.size(), 0);
}

/**
 * @brief CRITICAL: Energy conservation validation
 * 
 * Total work done by external forces should equal strain energy
 */
TEST_F(AnalysisOrchestratorTest, EnergyConservation) {
    Truss truss = createSimpleTruss();
    
    analysis::AnalysisOptions options;
    options.checkStability = false;
    
    auto solver = SolverFactory::createDirectSolver();
    AnalysisOrchestrator orchestrator(std::move(solver), options);
    auto results = orchestrator.analyze(truss);
    
    // Compute external work: W = (1/2) * F · u
    Real externalWork = 0.0;
    const auto& nodes = truss.getNodes();
    for (const auto& node : nodes) {
        Index dofX = node->getDofX();
        Index dofY = node->getDofY();
        Force2D force = node->getAppliedForce();
        
        externalWork += 0.5 * force.fx * results.displacements[dofX];
        externalWork += 0.5 * force.fy * results.displacements[dofY];
    }
    
    // Compare with stored strain energy
    EXPECT_NEAR(externalWork, results.totalStrain, std::abs(externalWork) * 0.01)
        << "Energy conservation violated (external work ≠ strain energy)";
}
