/**
 * @file test_simple_integration.cpp
 * @brief Google Test integration tests for truss creation without analysis
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Migration Notes:
 * - Converted from custom TestFramework.hpp to Google Test
 * - Preserved original test intent (truss creation and validation only)
 * - Tests structural properties without running analysis
 */

#include "../../src/core/model/truss.hpp"
#include "../../src/core/validation/truss_validator.hpp"

#include <gtest/gtest.h>

using namespace truss::core;

// Test Suite: SimpleIntegrationTest
// Tests for truss creation and structural validation without analysis

TEST(SimpleIntegrationTest, TrussCreationWithoutAnalysis) {
    // Test just creating and manipulating a truss without analysis
    Truss truss("Simple Test");

    // Add nodes
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerX);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);

    // Add members
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);

    // Apply load
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));

    // Verify truss is valid (using TrussValidator instead of deprecated methods)
    truss::core::validation::TrussValidator validator;
    EXPECT_TRUE(validator.isValid(truss));
    EXPECT_EQ(truss.getNodeCount(), 3);
    EXPECT_EQ(truss.getMemberCount(), 3);
}

// GTest provides main() automatically via GTest::gtest_main
// No manual main() needed
