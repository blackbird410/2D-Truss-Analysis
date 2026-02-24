/**
 * @file ianalysis_results_view.hpp
 * @brief Read-only view interface for AnalysisResults data.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This interface provides Infrastructure layer with read-only access to
 * analysis results, enforcing Dependency Inversion Principle (DIP).
 */

#pragma once

#include "../model/types.hpp"

#include <vector>

namespace truss::core::interfaces {

/**
 * @brief Read-only view interface for AnalysisResults
 *
 * This interface allows Infrastructure layer (exporters) to access
 * analysis results without depending on the concrete AnalysisResults struct.
 *
 * Benefits:
 * - Infrastructure depends on abstraction, not concrete type (DIP)
 * - Domain controls what data is exposed to Infrastructure
 * - Changes to AnalysisResults don't force Infrastructure recompilation
 */
class IAnalysisResultsView {
public:
    virtual ~IAnalysisResultsView() = default;

    // Displacement data
    virtual const std::vector<Real>& getDisplacements() const = 0;

    // Force data
    virtual const std::vector<Real>& getReactions() const = 0;
    virtual const std::vector<Real>& getMemberForces() const = 0;
    virtual const std::vector<Real>& getMemberStresses() const = 0;
    virtual const std::vector<Real>& getUtilizationRatios() const = 0;

    // Stiffness matrix
    virtual const std::vector<std::vector<Real>>& getStiffnessMatrix() const = 0;

    // Analysis metadata
    virtual bool hasConverged() const = 0;
    virtual int getIterations() const = 0;
    virtual Real getResidualNorm() const = 0;
    virtual Real getConditionNumber() const = 0;

    // System properties
    virtual size_t getTotalDofs() const = 0;
    virtual size_t getFreeDofs() const = 0;
    virtual size_t getConstrainedDofs() const = 0;
    virtual Real getTotalStrain() const = 0;
    virtual Real getMaxDisplacement() const = 0;
    virtual Real getMaxStress() const = 0;
};

}  // namespace truss::core::interfaces
