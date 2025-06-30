#include "../../src/core/Truss.hpp"
#include "../../src/core/AnalysisEngine.hpp"
#include <iostream>

using namespace truss::core;

int main() {
    std::cout << "=== Member Debug Test ===" << std::endl;
    
    // Create truss
    Truss truss("Member Debug");
    
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    std::cout << "After adding nodes: " << truss.getNodeCount() << " nodes" << std::endl;
    
    auto mem1 = truss.addMember(node1, node2);
    auto mem2 = truss.addMember(node1, node3);
    auto mem3 = truss.addMember(node2, node3);
    
    std::cout << "After adding members: " << truss.getMemberCount() << " members" << std::endl;
    
    for (size_t i = 0; i < truss.getMemberCount(); ++i) {
        auto members = truss.getMembers();
        std::cout << "Member " << i+1 << ": Length = " << members[i]->getLength() 
                  << ", Stiffness = " << members[i]->getStiffness() << std::endl;
    }
    
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    std::cout << "After applying load: " << truss.getMemberCount() << " members" << std::endl;
    
    truss.assignDofNumbers();
    std::cout << "After assigning DOFs: " << truss.getMemberCount() << " members" << std::endl;
    
    // Check statistics
    auto stats = truss.getStatistics();
    std::cout << "\nTruss Statistics:" << std::endl;
    std::cout << "Total nodes: " << stats.totalNodes << std::endl;
    std::cout << "Total members: " << stats.totalMembers << std::endl;
    std::cout << "Total length: " << stats.totalLength << std::endl;
    std::cout << "Free DOFs: " << stats.freeDofs << std::endl;
    std::cout << "Constrained DOFs: " << stats.constrainedDofs << std::endl;
    
    // Check member connectivity
    const auto& members = truss.getMembers();
    std::cout << "\nMember connectivity:" << std::endl;
    for (size_t i = 0; i < members.size(); ++i) {
        std::cout << "Member " << (i+1) << ": Node " 
                  << members[i]->getStartNode()->getId() << " -> Node "
                  << members[i]->getEndNode()->getId() << std::endl;
    }
    
    return 0;
}
