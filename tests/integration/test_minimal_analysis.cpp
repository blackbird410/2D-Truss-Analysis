/**
 * @file test_minimal_analysis.cpp
 * @brief Google Test minimal integration test for basic analysis workflow
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Migration Notes:
 * - Converted from minimal console application to GTest format
 * - Preserved basic triangular truss workflow
 * - Maintained console output for debugging (optional with --gtest_verbose)
 * - Tests absolute minimal successful analysis workflow
 */

#include <gtest/gtest.h>
#include <iostream>
#include "../../src/core/model/Truss.hpp"
#include "../../src/core/analysis/AnalysisOrchestrator.hpp"
#include "../../src/core/analysis/DirectSolver.hpp"

using namespace truss::core;

// Test Suite: MinimalAnalysisTest
// Minimal integration test for basic analysis workflow

TEST(MinimalAnalysisTest, BasicTriangularTrussWorkflow) {
    // Create truss
    Truss truss("Minimal Test");
    
    // Add a simple triangular truss
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    
    // Create analysis engine
    truss::core::analysis::AnalysisOrchestrator orchestrator(std::make_unique<truss::core::analysis::DirectSolver>(), std::make_unique<truss::core::validation::TrussValidator>());
    
    // Perform analysis
    try {
        truss::core::analysis::AnalysisResults results = orchestrator.analyze(truss);
        
        // Validate results
        EXPECT_TRUE(results.converged) << "Analysis should converge";
        EXPECT_GT(results.maxDisplacement, 0.0) << "Max displacement should be positive";
        
        // Optional: Print results for debugging (visible with --gtest_verbose)
        std::cout << "Analysis completed successfully!" << std::endl;
        std::cout << "Converged: " << results.converged << std::endl;
        std::cout << "Max displacement: " << results.maxDisplacement << std::endl;
        
    } catch (const std::exception& e) {
        FAIL() << "Analysis failed with exception: " << e.what();
    }
}

// GTest provides main() automatically via GTest::gtest_main
// No manual main() needed
