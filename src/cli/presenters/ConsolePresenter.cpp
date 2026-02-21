/**
 * @file ConsolePresenter.cpp
 * @brief Implementation of console presenter
 * 
 * Phase 5A.2: Refactored to eliminate direct console I/O.
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
    m_output.info("=================================================");
    m_output.info("       2D Truss Analysis Software v3.0.0       ");
    m_output.info("   Civil Engineering Software Solutions         ");
    m_output.info("=================================================");
    m_output.info(""); // Intentional spacing
}

void ConsolePresenter::displayTrussStatistics(const ITrussView& trussView) const {
    m_output.info("Truss Statistics:");
    m_output.info("  Name: " + trussView.getName());
    m_output.info("  Nodes: " + std::to_string(trussView.getNodeCount()));
    m_output.info("  Members: " + std::to_string(trussView.getMemberCount()));
    m_output.info("  Total DOFs: " + std::to_string(trussView.getTotalDofs()));
    m_output.info("  Free DOFs: " + std::to_string(trussView.getFreeDofs()));
    m_output.info("  Constrained DOFs: " + std::to_string(trussView.getConstrainedDofs()));
}

void ConsolePresenter::displayAnalysisResults(const IAnalysisResultsView& resultsView) const {
    m_output.info("");
    m_output.info("Analysis Results:");
    m_output.info("  Converged: " + std::string(resultsView.hasConverged() ? "Yes" : "No"));
    
    {
        std::ostringstream oss;
        oss << "  Maximum displacement: " << std::fixed << std::setprecision(4)
            << resultsView.getMaxDisplacement() * 1000 << " mm";
        m_output.info(oss.str());
    }
    
    {
        std::ostringstream oss;
        oss << "  Maximum stress: " << std::fixed << std::setprecision(2)
            << resultsView.getMaxStress() / 1e6 << " MPa";
        m_output.info(oss.str());
    }
    
    {
        std::ostringstream oss;
        oss << "  Total strain: " << std::scientific << std::setprecision(3)
            << resultsView.getTotalStrain();
        m_output.info(oss.str());
    }
    
    {
        std::ostringstream oss;
        oss << "  Matrix condition number: " << std::fixed << std::setprecision(1)
            << resultsView.getConditionNumber();
        m_output.info(oss.str());
    }
    
    // Display member forces
    const auto& memberForces = resultsView.getMemberForces();
    const auto& memberStresses = resultsView.getMemberStresses();
    const auto& utilizationRatios = resultsView.getUtilizationRatios();
    
    m_output.info("");
    m_output.info("Member Forces:");
    for (size_t i = 0; i < memberForces.size(); ++i) {
        Real force = memberForces[i];
        Real stress = memberStresses[i];
        Real utilization = utilizationRatios[i];
        
        std::ostringstream oss;
        oss << "  Member " << (i+1) 
            << ": Force = " << std::fixed << std::setprecision(2) << force/1000 << " kN"
            << ", Stress = " << std::fixed << std::setprecision(1) << stress/1e6 << " MPa"
            << ", Utilization = " << std::fixed << std::setprecision(1) << utilization*100 << "%"
            << (force > 0 ? " (Tension)" : " (Compression)");
        m_output.info(oss.str());
    }
    
    // Display node displacements
    const auto& displacements = resultsView.getDisplacements();
    m_output.info("");
    m_output.info("Node Displacements:");
    
    // Note: This is a simplified display. Full implementation would iterate through nodes.
    // For now, display first few DOFs as examples.
    size_t numDofs = std::min(displacements.size(), size_t(6)); // Show first 3 nodes (6 DOFs)
    for (size_t i = 0; i < numDofs; i += 2) {
        Real dispX = displacements[i] * 1000; // mm
        Real dispY = displacements[i+1] * 1000; // mm
        
        std::ostringstream oss;
        oss << "  Node " << (i/2 + 1) 
            << ": (" << std::fixed << std::setprecision(4) 
            << dispX << ", " << dispY << ") mm";
        m_output.info(oss.str());
    }
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
