/**
 * @file main_app.cpp
 * @brief Main application for 2D Truss Analysis
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "core/analysis/AnalysisOrchestrator.hpp"
#include "core/analysis/DirectSolver.hpp"
#include "core/model/Truss.hpp"
#include <iostream>
#include <fstream>
#include <string>

using namespace truss::core;

void printHeader() {
    std::cout << "=================================================\n";
    std::cout << "       2D Truss Analysis Software v3.0.0       \n";
    std::cout << "   Civil Engineering Software Solutions         \n";
    std::cout << "=================================================\n\n";
}

void printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  TrussAnalysis [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --verbose  Enable verbose output\n";
    std::cout << "  -e, --example  Run example analysis\n\n";
}

void runExampleAnalysis(bool verbose = false) {
    try {
        std::cout << "Running example truss analysis...\n\n";
        
        // Create example truss
        Truss truss("Example Truss");
        
        // Add nodes
        auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);   // Fixed support (X,Y constrained)
        auto node2 = truss.addNode(4.0, 0.0, SupportType::PinnedY);  // Roller support (Y constrained)
        auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);     // Free node
        
        // Define material properties (steel)
        MaterialProperties steel;
        steel.youngModulus = 200e9; // Pa
        steel.density = 7850;       // kg/m³
        steel.yieldStrength = 250e6; // Pa
        steel.name = "Steel S355";

        // Define section properties
        SectionProperties section;
        section.area = 2e-3;        // m² (20 cm²)
        section.designation = "L50x50x5";

        // Add members to stabilize as determinate structure
        truss.addMember(node1, node2, steel, section);
        truss.addMember(node1, node3, steel, section);
        truss.addMember(node2, node3, steel, section);
        
        // Apply loads
        truss.applyForce(node3->getId(), Force2D(0.0, -50000.0));   // 50 kN downward
        
        // Display truss information
        auto stats = truss.getStatistics();
        std::cout << "Truss Statistics:\n";
        std::cout << "  Nodes: " << stats.totalNodes << "\n";
        std::cout << "  Members: " << stats.totalMembers << "\n";
        std::cout << "  Total length: " << stats.totalLength << " m\n";
        std::cout << "  Total weight: " << stats.totalWeight << " kg\n";
        std::cout << "  Free DOFs: " << stats.freeDofs << "\n";
        std::cout << "  Constrained DOFs: " << stats.constrainedDofs << "\n\n";
        
        // Set up analysis options
        truss::core::analysis::AnalysisOptions options;
        options.verbose = verbose;
        options.useDirectSolver = true;
        options.computeReactions = true;
        
        // Perform analysis with AnalysisOrchestrator
        truss::core::analysis::AnalysisOrchestrator orchestrator(
            std::make_unique<truss::core::analysis::DirectSolver>(),
            options
        );
        
        std::cout << "Performing structural analysis...\n";
        auto results = orchestrator.analyze(truss);
        
        // Display results
        std::cout << "\nAnalysis Results:\n";
        std::cout << "  Converged: " << (results.converged ? "Yes" : "No") << "\n";
        std::cout << "  Maximum displacement: " << results.maxDisplacement * 1000 << " mm\n";
        std::cout << "  Maximum stress: " << results.maxStress / 1e6 << " MPa\n";
        std::cout << "  Total strain energy: " << results.totalStrain << " J\n";
        std::cout << "  Matrix condition number: " << results.conditionNumber << "\n\n";
        
        // Display member forces
        std::cout << "Member Forces:\n";
        const auto& members = truss.getMembers();
        for (size_t i = 0; i < members.size(); ++i) {
            Real force = results.memberForces[i];
            Real stress = results.memberStresses[i];
            Real utilization = results.utilizationRatios[i];
            
            std::cout << "  Member " << (i+1) 
                     << ": Force = " << force/1000 << " kN"
                     << ", Stress = " << stress/1e6 << " MPa"
                     << ", Utilization = " << utilization*100 << "%"
                     << (force > 0 ? " (Tension)" : " (Compression)")
                     << "\n";
        }
        
        // Display node displacements
        std::cout << "\nNode Displacements:\n";
        const auto& nodes = truss.getNodes();
        for (size_t i = 0; i < nodes.size(); ++i) {
            Index dofX = nodes[i]->getDofX();
            Index dofY = nodes[i]->getDofY();
            Real dispX = results.displacements[dofX] * 1000; // mm
            Real dispY = results.displacements[dofY] * 1000; // mm
            
            std::cout << "  Node " << (i+1) 
                     << ": (" << dispX << ", " << dispY << ") mm\n";
        }
        
        std::cout << "\nAnalysis completed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Analysis failed: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    printHeader();
    
    bool verbose = false;
    bool runExample = false;
    bool showHelp = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            showHelp = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-e" || arg == "--example") {
            runExample = true;
        }
    }
    
    if (showHelp) {
        printUsage();
        return 0;
    }
    
    if (runExample) {
        runExampleAnalysis(verbose);
        return 0;
    }
    
    // Default behavior - show usage
    std::cout << "Welcome to 2D Truss Analysis Software!\n\n";
    printUsage();
    
    std::cout << "Quick start: Run './TrussAnalysis -e' to see an example analysis.\n\n";
    
    return 0;
}
