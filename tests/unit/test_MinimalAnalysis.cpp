#include "../../src/core/Truss.hpp"
#include "../../src/core/AnalysisEngine.hpp"
#include <iostream>

using namespace truss::core;

int main() {
    std::cout << "Creating truss..." << std::endl;
    Truss truss("Minimal Test");
    
    // Add a simple triangular truss
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    
    std::cout << "Creating analysis engine..." << std::endl;
    AnalysisEngine engine;
    
    std::cout << "Starting analysis..." << std::endl;
    try {
        AnalysisResults results = engine.analyze(truss);
        std::cout << "Analysis completed successfully!" << std::endl;
        std::cout << "Converged: " << results.converged << std::endl;
        std::cout << "Max displacement: " << results.maxDisplacement << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Analysis failed: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Exiting program..." << std::endl;
    return 0;
}
