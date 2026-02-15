/**
 * @file ConsolePresenter.cpp
 * @brief Implementation of console presenter
 */

#include "ConsolePresenter.hpp"
#include <iomanip>

namespace truss::cli::presenters {

using truss::core::interfaces::ITrussView;
using truss::core::interfaces::IAnalysisResultsView;
using truss::core::Real;

void ConsolePresenter::displayHeader() const {
    std::cout << "=================================================\n";
    std::cout << "       2D Truss Analysis Software v3.0.0       \n";
    std::cout << "   Civil Engineering Software Solutions         \n";
    std::cout << "=================================================\n\n";
}

void ConsolePresenter::displayTrussStatistics(const ITrussView& trussView) const {
    std::cout << "Truss Statistics:\n";
    std::cout << "  Name: " << trussView.getName() << "\n";
    std::cout << "  Nodes: " << trussView.getNodeCount() << "\n";
    std::cout << "  Members: " << trussView.getMemberCount() << "\n";
    std::cout << "  Total DOFs: " << trussView.getTotalDofs() << "\n";
    std::cout << "  Free DOFs: " << trussView.getFreeDofs() << "\n";
    std::cout << "  Constrained DOFs: " << trussView.getConstrainedDofs() << "\n\n";
}

void ConsolePresenter::displayAnalysisResults(const IAnalysisResultsView& resultsView) const {
    std::cout << "\nAnalysis Results:\n";
    std::cout << "  Converged: " << (resultsView.hasConverged() ? "Yes" : "No") << "\n";
    std::cout << "  Maximum displacement: " << std::fixed << std::setprecision(4)
              << resultsView.getMaxDisplacement() * 1000 << " mm\n";
    std::cout << "  Maximum stress: " << std::fixed << std::setprecision(2)
              << resultsView.getMaxStress() / 1e6 << " MPa\n";
    std::cout << "  Total strain: " << std::scientific << std::setprecision(3)
              << resultsView.getTotalStrain() << "\n";
    std::cout << "  Matrix condition number: " << std::fixed << std::setprecision(1)
              << resultsView.getConditionNumber() << "\n\n";
    
    // Display member forces
    const auto& memberForces = resultsView.getMemberForces();
    const auto& memberStresses = resultsView.getMemberStresses();
    const auto& utilizationRatios = resultsView.getUtilizationRatios();
    
    std::cout << "Member Forces:\n";
    for (size_t i = 0; i < memberForces.size(); ++i) {
        Real force = memberForces[i];
        Real stress = memberStresses[i];
        Real utilization = utilizationRatios[i];
        
        std::cout << "  Member " << (i+1) 
                 << ": Force = " << std::fixed << std::setprecision(2) << force/1000 << " kN"
                 << ", Stress = " << std::fixed << std::setprecision(1) << stress/1e6 << " MPa"
                 << ", Utilization = " << std::fixed << std::setprecision(1) << utilization*100 << "%"
                 << (force > 0 ? " (Tension)" : " (Compression)")
                 << "\n";
    }
    
    // Display node displacements
    const auto& displacements = resultsView.getDisplacements();
    std::cout << "\nNode Displacements:\n";
    
    // Note: This is a simplified display. Full implementation would iterate through nodes.
    // For now, display first few DOFs as examples.
    size_t numDofs = std::min(displacements.size(), size_t(6)); // Show first 3 nodes (6 DOFs)
    for (size_t i = 0; i < numDofs; i += 2) {
        Real dispX = displacements[i] * 1000; // mm
        Real dispY = displacements[i+1] * 1000; // mm
        
        std::cout << "  Node " << (i/2 + 1) 
                 << ": (" << std::fixed << std::setprecision(4) 
                 << dispX << ", " << dispY << ") mm\n";
    }
}

void ConsolePresenter::displayError(const std::string& message) const {
    std::cerr << "ERROR: " << message << "\n";
}

void ConsolePresenter::displaySuccess(const std::string& message) const {
    std::cout << "SUCCESS: " << message << "\n";
}

void ConsolePresenter::displayInfo(const std::string& message) const {
    std::cout << "INFO: " << message << "\n";
}

} // namespace truss::cli::presenters
