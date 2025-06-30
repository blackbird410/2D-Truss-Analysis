#include "../TestFramework.hpp"
#include "../../src/core/Truss.hpp"

using namespace truss::core;
using namespace truss::testing;

void test_simple_truss_creation() {
    // Test just creating and manipulating a truss without analysis
    Truss truss("Simple Test");
    
    // Add nodes
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    // Add members
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    // Apply load
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    
    // Verify truss is valid
    ASSERT_TRUE(truss.isValid());
    ASSERT_TRUE(truss.isStaticallyDeterminate());
    ASSERT_EQ(truss.getNodeCount(), 3);
    ASSERT_EQ(truss.getMemberCount(), 3);
}

int main() {
    TestFramework framework;
    
    framework.beginSuite("Simple Integration Tests");
    
    framework.runTest("Simple truss creation without analysis", test_simple_truss_creation);
    
    framework.generateReport();
    
    return framework.allTestsPassed() ? 0 : 1;
}
