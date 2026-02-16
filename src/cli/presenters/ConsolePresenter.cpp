/**
 * @file ConsolePresenter.cpp
 * @brief Implementation of console presenter
 * 
 * Phase 5A.2: Refactored to eliminate direct I/O (std::cout/cerr).
 * All output now routed through IApplicationOutput abstraction.
 */

#include "ConsolePresenter.hpp"
#include <iomanip>
#include <sstream>

namespace truss::cli::presenters {

using truss::core::interfaces::ITrussView;
using truss::core::interfaces::IAnalysisResultsView;
using truss::core::Real;

ConsolePresenter::ConsolePresenter(application::interfaces::IApplicationOutput& output)
    : m_output(output) {
}

void ConsolePresenter::displayHeader() const {
    std::ostringstream oss;
    oss << "=================================================\n";
    oss << "       2D Truss Analysis Software v3.0.0       \n";
    oss << "   Civil Engineering Software Solutions         \n";
    oss << "=================================================\n";
    m_output.info(oss.str());
}

void ConsolePresenter::displayTrussStatistics(const ITrussView& trussView) const {
    std::ostringstream oss;
    oss << "Truss Statistics:\n";
    oss << "  Name: " << trussView.getName() << "\n";
    oss << "  Nodes: " << trussView.getNodeCount() << "\n";
    oss << "  Members: " << trussView.getMemberCount() << "\n";
    oss << "  Total DOFs: " << trussView.getTotalDofs() << "\n";
    oss << "  Free DOFs: " << trussView.getFreeDofs() << "\n";
    oss << "  Constrained DOFs: " << trussView.getConstrainedDofs() << "\n";
    m_output.info(oss.str());
}

void ConsolePresenter::displayAnalysisResults(const IAnalysisResultsView& resultsView) const {
    std::ostringstream oss;
    oss << "\nAnalysis Results:\n";
    oss << "  Converged: " << (resultsView.hasConverged() ? "Yes" : "No") << "\n";
    oss << "  Maximum displacement: " << std::fixed << std::setprecision(4)
        << resultsView.getMaxDisplacement() * 1000 << " mm\n";
    oss << "  Maximum stress: " << std::fixed << std::setprecision(2)
        << resultsView.getMaxStress() / 1e6 << " MPa\n";
    oss << "  Total strain: " << std::scientific << std::setprecision(3)
        << resultsView.getTotalStrain() << "\n";
    oss << "  Matrix condition number: " << std::fixed << std::setprecision(1)
        << resultsView.getConditionNumber() << "\n";
    
    // Display member forces
    const auto& memberForces = resultsView.getMemberForces();
    const auto& memberStresses = resultsView.getMemberStresses();
    const auto& utilizationRatios = resultsView.getUtilizationRatios();
    
    oss << "\nMember Forces:\n";
    for (size_t i = 0; i < memberForces.size(); ++i) {
        Real force = memberForces[i];
        Real stress = memberStresses[i];
        Real utilization = utilizationRatios[i];
        
        oss << "  Member " << (i+1) 
            << ": Force = " << std::fixed << std::setprecision(2) << force/1000 << " kN"
            << ", Stress = " << std::fixed << std::setprecision(1) << stress/1e6 << " MPa"
            << ", Utilization = " << std::fixed << std::setprecision(1) << utilization*100 << "%"
            << (force > 0 ? " (Tension)" : " (Compression)")
            << "\n";
    }
    
    // Display node displacements
    const auto& displacements = resultsView.getDisplacements();
    oss << "\nNode Displacements:\n";
    
    // Note: This is a simplified display. Full implementation would iterate through nodes.
    // For now, display first few DOFs as examples.
    size_t numDofs = std::min(displacements.size(), size_t(6)); // Show first 3 nodes (6 DOFs)
    for (size_t i = 0; i < numDofs; i += 2) {
        Real dispX = displacements[i] * 1000; // mm
        Real dispY = displacements[i+1] * 1000; // mm
        
        oss << "  Node " << (i/2 + 1) 
            << ": (" << std::fixed << std::setprecision(4) 
            << dispX << ", " << dispY << ") mm\n";
    }
    
    m_output.info(oss.str());
}

void ConsolePresenter::displayError(const std::string& message) const {
    m_output.error(message);
}

void ConsolePresenter::displaySuccess(const std::string& message) const {
    m_output.success(message);
}

void ConsolePresenter::displayInfo(const std::string& message) const {
    m_output.info(message);
}

} // namespace truss::cli::presenters
