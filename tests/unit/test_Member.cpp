#include "../TestFramework.hpp"
#include "../../src/core/Member.hpp"
#include "../../src/core/Node.hpp"

using namespace truss::core;
using namespace truss::testing;

void test_member_creation() {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 3.0));
    MaterialProperties material;
    SectionProperties section;
    Member member(1, node1, node2, material, section);
    
    ASSERT_EQ(member.getId(), 1);
    ASSERT_EQ(member.getStartNode(), node1);
    ASSERT_EQ(member.getEndNode(), node2);
    ASSERT_EQ(member.getLength(), 5.0);
}

void test_member_stiffness_and_weight() {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(4.0, 3.0));
    MaterialProperties material(200e9, 7850.0, 250e6, 400e6, "Steel"); // E, rho, fy, fu, name
    SectionProperties section(1e-4, 1e-8, 1e-4, "Default");            // A, I, As, designation
    Member member(1, node1, node2, material, section);
    
    // Expected stiffness = EA/L = 200e9 * 1e-4 / 5.0 = 4e6
    ASSERT_NEAR(member.getStiffness(), 4e6, 1e-9);
    // Expected weight = density * area * length = 7850 * 1e-4 * 5.0 = 3.925
    ASSERT_NEAR(member.getWeight(), 3.925, 1e-3);
}

void test_member_geometric_checks() {
    auto node1 = std::make_shared<Node>(1, Point2D(0.0, 0.0));
    auto node2 = std::make_shared<Node>(2, Point2D(10.0, 0.0));
    Member member(1, node1, node2);
    
    ASSERT_TRUE(member.isHorizontal());
    ASSERT_FALSE(member.isVertical());
}

int main() {
    TestFramework framework;
    
    framework.beginSuite("Member Class Tests");
    
    framework.runTest("Member creation and basic properties", test_member_creation);
    framework.runTest("Stiffness and weight calculations", test_member_stiffness_and_weight);
    framework.runTest("Geometric checks (horizontal/vertical)", test_member_geometric_checks);
    
    framework.generateReport();
    
    return framework.allTestsPassed() ? 0 : 1;
}

