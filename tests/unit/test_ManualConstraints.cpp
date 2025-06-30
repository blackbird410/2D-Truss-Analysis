#include "../../src/core/Truss.hpp"
#include <iostream>

using namespace truss::core;

int main() {
    // Create the same truss
    Truss truss("Manual Constraints");
    
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);     // DOFs 0,1 - both constrained
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);    // DOFs 2,3 - X constrained, Y free
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);       // DOFs 4,5 - both free
    
    truss.assignDofNumbers();
    
    std::cout << "Manual constraint analysis:" << std::endl;
    std::cout << "Node 1 (Pinned): X=" << node1->getDofX() << "(constrained), Y=" << node1->getDofY() << "(constrained)" << std::endl;
    std::cout << "Node 2 (RollerY): X=" << node2->getDofX() << "(constrained), Y=" << node2->getDofY() << "(free)" << std::endl;
    std::cout << "Node 3 (Free): X=" << node3->getDofX() << "(free), Y=" << node3->getDofY() << "(free)" << std::endl;
    
    std::cout << "\nExpected free DOFs: " << node2->getDofY() << ", " << node3->getDofX() << ", " << node3->getDofY() << std::endl;
    std::cout << "Expected constrained DOFs: " << node1->getDofX() << ", " << node1->getDofY() << ", " << node2->getDofX() << std::endl;
    
    // Check if structure is statically determinate
    size_t nodes = truss.getNodeCount();       // 3 nodes
    size_t members = truss.getMemberCount();   // 3 members
    size_t reactions = 3;                      // 2 for pinned + 1 for roller = 3 reaction components
    
    std::cout << "\nStatic determinacy check:" << std::endl;
    std::cout << "Nodes: " << nodes << std::endl;
    std::cout << "Members: " << members << std::endl;
    std::cout << "Reactions: " << reactions << std::endl;
    std::cout << "Equations of equilibrium: " << 2 * nodes << " (2 per node)" << std::endl;
    std::cout << "Total constraints: " << members + reactions << std::endl;
    std::cout << "DOF = " << 2 * nodes - reactions << " = " << (2 * nodes - reactions) << std::endl;
    
    if (members + reactions == 2 * nodes) {
        std::cout << "Structure is statically determinate!" << std::endl;
    } else if (members + reactions > 2 * nodes) {
        std::cout << "Structure is statically indeterminate!" << std::endl;
    } else {
        std::cout << "Structure is unstable!" << std::endl;
    }
    
    return 0;
}
