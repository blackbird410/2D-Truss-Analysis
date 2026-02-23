/**
 * @file analysis_results.hpp
 * @brief Data structures for analysis results
 */

#pragma once

#include <vector>
#include <map>
#include <cstdint>

namespace truss {

// Forward declarations
using NodeId = std::uint32_t;
using MemberId = std::uint32_t;

/**
 * @struct AnalysisResults
 * @brief Container for results from structural analysis
 * 
 * Results include:
 * - Node displacements (Δx, Δy)
 * - Reaction forces at supports
 * - Member internal forces (axial)
 * - Member stresses
 * - Strain energy
 */
struct AnalysisResults {
    // Node displacements: nodeId -> (displacement_x, displacement_y)
    std::map<NodeId, std::pair<double, double>> nodeDisplacements;
    
    // Reaction forces: nodeId -> (force_x, force_y)
    std::map<NodeId, std::pair<double, double>> reactionForces;
    
    // Member internal forces: memberId -> axial force
    std::map<MemberId, double> memberForces;
    
    // Member stresses: memberId -> stress
    std::map<MemberId, double> memberStresses;
    
    // Total strain energy
    double strainEnergy = 0.0;
    
    // Convergence flag
    bool converged = false;
    
    // Number of iterations (for iterative solvers)
    int iterations = 0;
};

} // namespace truss
