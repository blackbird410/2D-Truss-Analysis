/**
 * @file analysis_orchestrator.hpp
 * @brief Orchestrates the structural analysis workflow using decomposed components.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "../interfaces/ianalysis_results_view.hpp"
#include "../model/truss.hpp"
#include "../model/types.hpp"
#include "../validation/truss_validator.hpp"
#include "boundary_condition_handler.hpp"
#include "ilinear_solver.hpp"
#include "stiffness_assembler.hpp"

#include <memory>
#include <vector>

namespace truss::core::analysis {

/**
 * @brief Analysis configuration options
 */
struct AnalysisOptions {
    Real convergenceTolerance{1e-9};  ///< Convergence tolerance for iterative solvers
    int maxIterations{1000};          ///< Maximum iterations for iterative solvers
    bool useDirectSolver{true};       ///< Use direct solver instead of iterative
    bool computeReactions{true};      ///< Compute support reactions
    bool checkStability{true};        ///< Check structural stability
    bool verbose{false};              ///< Enable verbose output
};

/**
 * @brief Complete analysis results for the truss system
 *
 * Implements IAnalysisResultsView to provide read-only access to Infrastructure layer.
 */
struct AnalysisResults : public interfaces::IAnalysisResultsView {
    std::vector<Real> displacements;      ///< Global displacement vector
    std::vector<Real> reactions;          ///< Support reaction forces
    std::vector<Real> memberForces;       ///< Member axial forces
    std::vector<Real> memberStresses;     ///< Member axial stresses
    std::vector<Real> utilizationRatios;  ///< Member utilization ratios

    // Stiffness matrix for display and analysis
    std::vector<std::vector<Real>> stiffnessMatrix;  ///< Global stiffness matrix

    // Analysis metadata
    bool converged{false};      ///< Analysis convergence status
    int iterations{0};          ///< Number of iterations performed
    Real residualNorm{0.0};     ///< Final residual norm
    Real conditionNumber{0.0};  ///< Matrix condition number

    // System properties
    size_t totalDofs{0};        ///< Total degrees of freedom
    size_t freeDofs{0};         ///< Free degrees of freedom
    size_t constrainedDofs{0};  ///< Constrained degrees of freedom
    Real totalStrain{0.0};      ///< Total strain energy
    Real maxDisplacement{0.0};  ///< Maximum displacement magnitude
    Real maxStress{0.0};        ///< Maximum stress in any member

    AnalysisResults() = default;

    // IAnalysisResultsView interface implementation
    const std::vector<Real>& getDisplacements() const override { return displacements; }
    const std::vector<Real>& getReactions() const override { return reactions; }
    const std::vector<Real>& getMemberForces() const override { return memberForces; }
    const std::vector<Real>& getMemberStresses() const override { return memberStresses; }
    const std::vector<Real>& getUtilizationRatios() const override { return utilizationRatios; }
    const std::vector<std::vector<Real>>& getStiffnessMatrix() const override {
        return stiffnessMatrix;
    }

    bool hasConverged() const override { return converged; }
    int getIterations() const override { return iterations; }
    Real getResidualNorm() const override { return residualNorm; }
    Real getConditionNumber() const override { return conditionNumber; }

    size_t getTotalDofs() const override { return totalDofs; }
    size_t getFreeDofs() const override { return freeDofs; }
    size_t getConstrainedDofs() const override { return constrainedDofs; }
    Real getTotalStrain() const override { return totalStrain; }
    Real getMaxDisplacement() const override { return maxDisplacement; }
    Real getMaxStress() const override { return maxStress; }
};

/**
 * @brief Orchestrates the complete structural analysis workflow
 *
 * The AnalysisOrchestrator coordinates the analysis process by delegating to specialized
 * components:
 * - StiffnessAssembler: Assembles global stiffness matrix
 * - BoundaryConditionHandler: Applies boundary conditions
 * - ILinearSolver: Solves the linear system
 *
 * This design follows the Strategy pattern for the solver and provides a clean,
 * testable separation of concerns.
 *
 * Example usage:
 * @code
 * auto solver = SolverFactory::createDirectSolver();
 * AnalysisOrchestrator orchestrator(std::move(solver));
 *
 * Truss truss;
 * // ... configure truss ...
 *
 * AnalysisResults results = orchestrator.analyze(truss);
 * @endcode
 */
class AnalysisOrchestrator {
public:
    /**
     * @brief Construct orchestrator with required dependencies
     * @param solver Solver strategy for the linear system
     * @param validator Validation service for structural integrity
     * @param options Analysis configuration options
     */
    explicit AnalysisOrchestrator(std::unique_ptr<ILinearSolver> solver,
                                  std::unique_ptr<validation::TrussValidator> validator,
                                  const AnalysisOptions& options = AnalysisOptions());

    /**
     * @brief Perform complete structural analysis
     * @param truss Truss structure to analyze
     * @return Complete analysis results
     * @throws std::runtime_error If analysis fails
     */
    AnalysisResults analyze(Truss& truss);

    /**
     * @brief Get the last analysis results
     * @return Last analysis results
     */
    [[maybe_unused]] const AnalysisResults& getLastResults() const noexcept {
        return m_lastResults;
    }

private:
    // Components (dependency injection)
    std::unique_ptr<StiffnessAssembler> m_assembler;
    std::unique_ptr<BoundaryConditionHandler> m_bcHandler;
    std::unique_ptr<ILinearSolver> m_solver;
    std::unique_ptr<validation::TrussValidator> m_validator;

    // Configuration
    AnalysisOptions m_options;

    // State
    AnalysisResults m_lastResults;

    // Workflow steps

    /**
     * @brief Assign degree-of-freedom indices to nodes
     * @param truss Truss structure
     */
    static void assignDOFs(Truss& truss);

    /**
     * @brief Assemble load vector from nodal forces
     * @param truss Truss structure
     * @return Global load vector
     */
    static VectorXd assembleLoadVector(const Truss& truss);

    /**
     * @brief Compute member axial forces from displacements
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @return Member axial forces
     */
    static std::vector<Real> computeMemberForces(const Truss& truss, const VectorXd& displacements);

    /**
     * @brief Compute support reactions from displacements
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @param K Global stiffness matrix
     * @return Reaction forces at constrained DOFs
     */
    VectorXd
    computeReactions(const Truss& truss, const VectorXd& displacements, const MatrixXd& K) const;

    /**
     * @brief Post-process results (compute stresses, utilization, etc.)
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @param K Global stiffness matrix
     * @return Complete analysis results
     */
    AnalysisResults
    postProcessResults(const Truss& truss, const VectorXd& displacements, const MatrixXd& K) const;

    /**
     * @brief Compute condition number of stiffness matrix
     * @param K Stiffness matrix
     * @return Condition number
     */
    static Real computeConditionNumber(const MatrixXd& K);

    /**
     * @brief Check for singular stiffness matrix
     * @param K Reduced stiffness matrix (after applying boundary conditions)
     * @return Determinant or matrix rank indicator
     * @throws std::runtime_error if matrix is singular
     */
    static Real checkMatrixSingularity(const MatrixXd& K);

    /**
     * @brief Compute strain energy from displacements
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @return Total strain energy
     */
    static Real computeStrainEnergy(const Truss& truss, const VectorXd& displacements);

    /**
     * @brief Find maximum displacement magnitude
     * @param displacements Global displacement vector
     * @return Maximum displacement magnitude
     */
    static Real findMaxDisplacement(const VectorXd& displacements);

    /**
     * @brief Find maximum stress in any member
     * @param stresses Member stresses
     * @return Maximum stress magnitude
     */
    static Real findMaxStress(const std::vector<Real>& stresses);

    /**
     * @brief Update truss with analysis results
     * @param truss Truss structure
     * @param results Analysis results
     */
    static void updateTrussResults(Truss& truss, const AnalysisResults& results);

    // Logging
    void logAnalysisStart(const Truss& truss) const;
    void logAnalysisComplete(const AnalysisResults& results) const;
};

}  // namespace truss::core::analysis
