/**
 * @file test_member.cpp
 * @brief Google Test unit tests for Member class
 * @author Refactoring Agent (migrated from custom framework)
 * @version 3.0.0-dev
 * 
 * Migration Notes:
 * - Converted from custom TestFramework.hpp to Google Test
 * - Preserved all original test intent and coverage
 * - Maintained numerical tolerances (1e-9 for stiffness, 1e-3 for weight)
 * - Removed manual test registration (GTest auto-discovers tests)
 */

#include <gtest/gtest.h>
#include <memory>
#include "../../src/core/Member.hpp"
#include "../../src/core/Node.hpp"

using namespace truss::core;

// Test Suite: MemberTest
// Tests for Member class creation, properties, and calculations

TEST(MemberTest, CreationAndBasicProperties) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 3.0));
    MaterialProperties material;
    SectionProperties section;
    Member member(1, node1, node2, material, section);
    
    EXPECT_EQ(member.getId(), 1);
    EXPECT_EQ(member.getStartNode(), node1);
    EXPECT_EQ(member.getEndNode(), node2);
    EXPECT_DOUBLE_EQ(member.getLength(), 5.0);
}

TEST(MemberTest, StiffnessAndWeightCalculations) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 3.0));
    MaterialProperties material(200e9, 7850.0, 250e6, 400e6, "Steel"); // E, rho, fy, fu, name
    SectionProperties section(1e-4, 1e-8, 1e-4, "Default");            // A, I, As, designation
    Member member(1, node1, node2, material, section);
    
    // Expected stiffness = EA/L = 200e9 * 1e-4 / 5.0 = 4e6
    EXPECT_NEAR(member.getStiffness(), 4e6, 1e-9);
    // Expected weight = density * area * length = 7850 * 1e-4 * 5.0 = 3.925
    EXPECT_NEAR(member.getWeight(), 3.925, 1e-3);
}

TEST(MemberTest, GeometricChecks) {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(10.0, 0.0));
    Member member(1, node1, node2);
    
    EXPECT_TRUE(member.isHorizontal());
    EXPECT_FALSE(member.isVertical());
}

// GTest provides main() automatically via GTest::gtest_main
// No manual main() needed
