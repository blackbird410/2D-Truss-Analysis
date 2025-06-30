/**
 * @file test_AnalysisEngine.cpp
 * @brief Unit tests for AnalysisEngine class
 */

#include "AnalysisEngine.hpp"
#include "Truss.hpp"
#include "TestFramework.hpp"
#include <iostream>

using namespace truss::core;
using namespace truss::testing;

void test_analysis_convergence() {
    // Setup a standard triangular truss for testing
    Truss truss("Test Truss");
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);

    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);

    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));

    // Perform the analysis
    AnalysisEngine engine;
    AnalysisResults results = engine.analyze(truss);

    // Assert analysis convergence
    ASSERT_TRUE(results.converged);

    // Check specific expected values with small tolerance
    ASSERT_NEAR(results.maxDisplacement, 0.002, 0.001);
    ASSERT_NEAR(results.maxStress, 70500.0, 100.0);
}

int main() {
    TestFramework framework;

    framework.beginSuite("Analysis Engine Tests");
    framework.runTest("Test convergence and results of standard truss", test_analysis_convergence);

    // Add more test cases here...

    framework.generateReport();

    return framework.allTestsPassed() ? 0 : 1;
}
