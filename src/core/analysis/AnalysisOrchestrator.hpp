/**
 * @file AnalysisOrchestrator.hpp
 * @brief Orchestrates the structural analysis workflow using decomposed components
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "../model/Types.hpp"
#include "../model/Truss.hpp"
#include "StiffnessAssembler.hpp"
#include "BoundaryConditionHandler.hpp"
#include "ILinearSolver.hpp"
#include <memory>
#include <vector>

namespace truss::core::analysis {

/**
 * @brief Analysis configuration options
 */
struct AnalysisOptions {
    Real convergenceTolerance{1e-9};    ///< Convergence tolerance for iterative solvers
    int maxIterations{1000};            ///< Maximum iterations for iterative solvers
    bool useDirectSolver{true};         ///< Use direct solver instead of iterative
    bool computeReactions{true};        ///< Compute support reactions
    bool checkStability{true};          ///< Check structural stability
    bool verbose{false};                ///< Enable verbose output
};

/**
 * @brief Complete analysis results for the truss system
 */
struct AnalysisResults {
    std::vector<Real> displacements;           ///< Global displacement vector
    std::vector<Real> reactions;               ///< Support reaction forces
    std::vector<Real> memberForces;            ///< Member axial forces
    std::vector<Real> memberStresses;          ///< Member axial stresses
    std::vector<Real> utilizationRatios;       ///< Member utilization ratios
    
    // Stiffness matrix for display and analysis
    std::vector<std::vector<Real>> stiffnessMatrix;  ///< Global stiffness matrix
    
    // Analysis metadata
    bool converged{false};                     ///< Analysis convergence status
    int iterations{0};                         ///< Number of iterations performed
    Real residualNorm{0.0};                   ///< Final residual norm
    Real conditionNumber{0.0};                ///< Matrix condition number
    
    // System properties
    size_t totalDofs{0};                      ///< Total degrees of freedom
    size_t freeDofs{0};                       ///< Free degrees of freedom
    size_t constrainedDofs{0};                ///< Constrained degrees of freedom
    Real totalStrain{0.0};                   ///< Total strain energy
    Real maxDisplacement{0.0};               ///< Maximum displacement magnitude
    Real maxStress{0.0};                     ///< Maximum stress in any member
    
    AnalysisResults() = default;
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
     * @brief Construct orchestrator with a linear solver
     * @param solver Solver strategy for the linear system
     * @param options Analysis configuration options
     */
    explicit AnalysisOrchestrator(
        std::unique_ptr<ILinearSolver> solver,
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
    const AnalysisResults& getLastResults() const noexcept { return m_lastResults; }

private:
    // Components (dependency injection)
    std::unique_ptr<StiffnessAssembler> m_assembler;
    std::unique_ptr<BoundaryConditionHandler> m_bcHandler;
    std::unique_ptr<ILinearSolver> m_solver;
    
    // Configuration
    AnalysisOptions m_options;
    
    // State
    AnalysisResults m_lastResults;
    
    // Workflow steps
    
    /**
     * @brief Assign degree-of-freedom indices to nodes
     * @param truss Truss structure
     */
    void assignDOFs(Truss& truss);
    
    /**
     * @brief Validate inputs before analysis
     * @param truss Truss structure
     * @return true if valid, false otherwise
     */
    bool validateInputs(const Truss& truss) const;
    
    /**
     * @brief Check structural validity (stability, determinacy)
     * @param truss Truss structure
     * @return true if structurally valid, false otherwise
     */
    bool checkStructuralValidity(const Truss& truss) const;
    
    /**
     * @brief Assemble load vector from nodal forces
     * @param truss Truss structure
     * @return Global load vector
     */
    VectorXd assembleLoadVector(const Truss& truss) const;
    
    /**
     * @brief Compute member axial forces from displacements
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @return Member axial forces
     */
    std::vector<Real> computeMemberForces(
        const Truss& truss, 
        const VectorXd& displacements) const;
    
    /**
     * @brief Compute support reactions from displacements
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @param K Global stiffness matrix
     * @return Reaction forces at constrained DOFs
     */
    VectorXd computeReactions(
        const Truss& truss,
        const VectorXd& displacements,
        const MatrixXd& K) const;
    
    /**
     * @brief Post-process results (compute stresses, utilization, etc.)
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @param K Global stiffness matrix
     * @return Complete analysis results
     */
    AnalysisResults postProcessResults(
        Truss& truss,
        const VectorXd& displacements,
        const MatrixXd& K);
    
    /**
     * @brief Compute condition number of stiffness matrix
     * @param K Stiffness matrix
     * @return Condition number
     */
    Real computeConditionNumber(const MatrixXd& K) const;
    
    /**
     * @brief Compute strain energy from displacements
     * @param truss Truss structure
     * @param displacements Global displacement vector
     * @return Total strain energy
     */
    Real computeStrainEnergy(const Truss& truss, const VectorXd& displacements) const;
    
    /**
     * @brief Find maximum displacement magnitude
     * @param displacements Global displacement vector
     * @return Maximum displacement magnitude
     */
    Real findMaxDisplacement(const VectorXd& displacements) const;
    
    /**
     * @brief Find maximum stress in any member
     * @param stresses Member stresses
     * @return Maximum stress magnitude
     */
    Real findMaxStress(const std::vector<Real>& stresses) const;
    
    /**
     * @brief Update truss with analysis results
     * @param truss Truss structure
     * @param results Analysis results
     */
    void updateTrussResults(Truss& truss, const AnalysisResults& results);
    
    // Logging
    void logAnalysisStart(const Truss& truss) const;
    void logAnalysisComplete(const AnalysisResults& results) const;
};

} // namespace truss::core::analysis
