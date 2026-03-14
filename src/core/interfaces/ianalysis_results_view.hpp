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

    // ================================================================
    // Displacement data
    // ================================================================

    /**
     * @brief Global nodal displacement vector, indexed by DOF.
     * @return Reference to flat vector of displacements (metres), ordered by global DOF index.
     */
    virtual const std::vector<Real>& getDisplacements() const = 0;

    // ================================================================
    // Force data
    // ================================================================

    /**
     * @brief Support reaction force vector, indexed by constrained DOF.
     * @return Reference to flat vector of reactions (N), ordered by global DOF index.
     */
    virtual const std::vector<Real>& getReactions() const = 0;

    /**
     * @brief Axial force in every member, indexed by member sequence.
     * @return Reference to vector of axial forces (N); positive = tension.
     */
    virtual const std::vector<Real>& getMemberForces() const = 0;

    /**
     * @brief Axial stress in every member, indexed by member sequence.
     * @return Reference to vector of axial stresses (Pa); positive = tension.
     */
    virtual const std::vector<Real>& getMemberStresses() const = 0;

    /**
     * @brief Utilisation ratio (σ / f_y) for every member, indexed by member sequence.
     * @return Reference to vector of utilisation ratios (dimensionless); values > 1 indicate
     *         yielding.
     */
    virtual const std::vector<Real>& getUtilizationRatios() const = 0;

    // ================================================================
    // Stiffness matrix
    // ================================================================

    /**
     * @brief Assembled global stiffness matrix.
     * @return Reference to the stiffness matrix as a row-major 2-D vector (N/m).
     *         Outer index is row, inner index is column.
     */
    virtual const std::vector<std::vector<Real>>& getStiffnessMatrix() const = 0;

    // ================================================================
    // Analysis metadata
    // ================================================================

    /**
     * @brief Whether the solver reached a converged solution.
     * @return true if the linear system converged within the configured tolerance.
     */
    virtual bool hasConverged() const = 0;

    /**
     * @brief Number of solver iterations performed (iterative solvers only).
     * @return Iteration count; always 1 for direct solvers.
     */
    virtual int getIterations() const = 0;

    /**
     * @brief L2 norm of the residual ‖Ax − b‖ after solution.
     * @return Residual norm (N or N/m depending on formulation); 0.0 before analysis.
     */
    virtual Real getResidualNorm() const = 0;

    /**
     * @brief Condition number of the assembled stiffness matrix.
     * @return Condition number (dimensionless); large values indicate ill-conditioning.
     */
    virtual Real getConditionNumber() const = 0;

    // ================================================================
    // System properties
    // ================================================================

    /**
     * @brief Total number of degrees of freedom in the system.
     * @return 2 × (number of nodes).
     */
    virtual size_t getTotalDofs() const = 0;

    /**
     * @brief Number of unconstrained (free) degrees of freedom.
     * @return Total DOFs minus constrained DOFs.
     */
    virtual size_t getFreeDofs() const = 0;

    /**
     * @brief Number of constrained (supported) degrees of freedom.
     * @return Count of DOFs eliminated by boundary condition application.
     */
    virtual size_t getConstrainedDofs() const = 0;

    /**
     * @brief Total elastic strain energy stored in the structure.
     * @return Strain energy (J).
     */
    virtual Real getTotalStrain() const = 0;

    /**
     * @brief Maximum nodal displacement magnitude across all nodes.
     * @return Maximum ‖u‖ (metres).
     */
    virtual Real getMaxDisplacement() const = 0;

    /**
     * @brief Maximum absolute axial stress across all members.
     * @return Maximum |σ| (Pa).
     */
    virtual Real getMaxStress() const = 0;
};

}  // namespace truss::core::interfaces
