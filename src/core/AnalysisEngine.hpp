/**
 * @file AnalysisEngine.hpp
 * @brief Analysis engine for structural analysis of 2D trusses
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
 */

#pragma once

#include "Types.hpp"
#include "Truss.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <memory>
#include <vector>

namespace truss::core {

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
    
    // Constructor
    AnalysisResults() = default;
    
    // Copy constructor
    AnalysisResults(const AnalysisResults& other)
        : displacements(other.displacements)
        , reactions(other.reactions)
        , memberForces(other.memberForces)
        , memberStresses(other.memberStresses)
        , utilizationRatios(other.utilizationRatios)
        , stiffnessMatrix(other.stiffnessMatrix)
        , converged(other.converged)
        , iterations(other.iterations)
        , residualNorm(other.residualNorm)
        , conditionNumber(other.conditionNumber)
        , totalDofs(other.totalDofs)
        , freeDofs(other.freeDofs)
        , constrainedDofs(other.constrainedDofs)
        , totalStrain(other.totalStrain)
        , maxDisplacement(other.maxDisplacement)
        , maxStress(other.maxStress) {}
    
    // Move constructor
    AnalysisResults(AnalysisResults&& other) noexcept
        : displacements(std::move(other.displacements))
        , reactions(std::move(other.reactions))
        , memberForces(std::move(other.memberForces))
        , memberStresses(std::move(other.memberStresses))
        , utilizationRatios(std::move(other.utilizationRatios))
        , stiffnessMatrix(std::move(other.stiffnessMatrix))
        , converged(other.converged)
        , iterations(other.iterations)
        , residualNorm(other.residualNorm)
        , conditionNumber(other.conditionNumber)
        , totalDofs(other.totalDofs)
        , freeDofs(other.freeDofs)
        , constrainedDofs(other.constrainedDofs)
        , totalStrain(other.totalStrain)
        , maxDisplacement(other.maxDisplacement)
        , maxStress(other.maxStress) {}
    
    // Assignment operators
    AnalysisResults& operator=(const AnalysisResults& other) {
        if (this != &other) {
            displacements = other.displacements;
            reactions = other.reactions;
            memberForces = other.memberForces;
            memberStresses = other.memberStresses;
            utilizationRatios = other.utilizationRatios;
            stiffnessMatrix = other.stiffnessMatrix;
            converged = other.converged;
            iterations = other.iterations;
            residualNorm = other.residualNorm;
            conditionNumber = other.conditionNumber;
            totalDofs = other.totalDofs;
            freeDofs = other.freeDofs;
            constrainedDofs = other.constrainedDofs;
            totalStrain = other.totalStrain;
            maxDisplacement = other.maxDisplacement;
            maxStress = other.maxStress;
        }
        return *this;
    }
    
    AnalysisResults& operator=(AnalysisResults&& other) noexcept {
        if (this != &other) {
            displacements = std::move(other.displacements);
            reactions = std::move(other.reactions);
            memberForces = std::move(other.memberForces);
            memberStresses = std::move(other.memberStresses);
            utilizationRatios = std::move(other.utilizationRatios);
            stiffnessMatrix = std::move(other.stiffnessMatrix);
            converged = other.converged;
            iterations = other.iterations;
            residualNorm = other.residualNorm;
            conditionNumber = other.conditionNumber;
            totalDofs = other.totalDofs;
            freeDofs = other.freeDofs;
            constrainedDofs = other.constrainedDofs;
            totalStrain = other.totalStrain;
            maxDisplacement = other.maxDisplacement;
            maxStress = other.maxStress;
        }
        return *this;
    }
    
    // Destructor
    ~AnalysisResults() = default;
};

/**
 * @brief Analysis engine for 2D truss structures
 * 
 * This class provides comprehensive structural analysis capabilities including
 * stiffness matrix assembly, load vector construction, equation solving,
 * and post-processing of results.
 */
class AnalysisEngine {
public:
    /**
     * @brief Construct an analysis engine
     * @param options Analysis configuration options
     */
    explicit AnalysisEngine(const AnalysisOptions& options = AnalysisOptions{});
    
    // Copy and move constructors
    AnalysisEngine(const AnalysisEngine& other) = default;
    AnalysisEngine(AnalysisEngine&& other) noexcept = default;
    
    // Assignment operators
    AnalysisEngine& operator=(const AnalysisEngine& other) = default;
    AnalysisEngine& operator=(AnalysisEngine&& other) noexcept = default;
    
    // Destructor
    ~AnalysisEngine() = default;
    
    /**
     * @brief Perform complete structural analysis
     * @param truss The truss structure to analyze
     * @return Analysis results
     */
    AnalysisResults analyze(Truss& truss);
    
    /**
     * @brief Assemble global stiffness matrix
     * @param truss The truss structure
     * @return Global stiffness matrix
     */
    MatrixXd assembleStiffnessMatrix(const Truss& truss);
    
    /**
     * @brief Assemble global load vector
     * @param truss The truss structure
     * @return Global load vector
     */
    VectorXd assembleLoadVector(const Truss& truss);
    
    /**
     * @brief Solve the system of linear equations K*u = F
     * @param K Global stiffness matrix
     * @param F Global load vector
     * @param truss Truss structure (for DOF information)
     * @return Solution vector (displacements)
     */
    VectorXd solveSystem(const MatrixXd& K, const VectorXd& F, const Truss& truss);
    
    /**
     * @brief Compute member forces from nodal displacements
     * @param truss The truss structure
     * @param displacements Global displacement vector
     * @return Vector of member axial forces
     */
    std::vector<Real> computeMemberForces(const Truss& truss, const VectorXd& displacements);
    
    /**
     * @brief Compute support reactions
     * @param truss The truss structure
     * @param displacements Global displacement vector
     * @return Support reaction vector
     */
    VectorXd computeReactions(const Truss& truss, const VectorXd& displacements);
    
    /**
     * @brief Update truss with analysis results
     * @param truss The truss structure to update
     * @param results Analysis results
     */
    void updateTrussResults(Truss& truss, const AnalysisResults& results);
    
    /**
     * @brief Check structural stability and determinacy
     * @param truss The truss structure
     * @return true if structure is stable and solvable
     */
    bool checkStructuralValidity(const Truss& truss);
    
    /**
     * @brief Get analysis options
     */
    const AnalysisOptions& getOptions() const noexcept { return m_options; }
    
    /**
     * @brief Set analysis options
     */
    void setOptions(const AnalysisOptions& options) { m_options = options; }
    
    /**
     * @brief Get last analysis results
     */
    const AnalysisResults& getLastResults() const noexcept { return m_lastResults; }
    
    // Debug interface for unit testing
    #ifdef DEBUG_TESTING
    std::vector<Index> getFreeDofIndicesPublic(const Truss& truss) { return getFreeDofIndices(truss); }
    std::vector<Index> getConstrainedDofIndicesPublic(const Truss& truss) { return getConstrainedDofIndices(truss); }
    MatrixXd extractFreeStiffnessMatrixPublic(const MatrixXd& K, const std::vector<Index>& freeDofs) { return extractFreeStiffnessMatrix(K, freeDofs); }
    VectorXd extractFreeLoadVectorPublic(const VectorXd& F, const std::vector<Index>& freeDofs) { return extractFreeLoadVector(F, freeDofs); }
    VectorXd expandSolutionVectorPublic(const VectorXd& freeSolution, const std::vector<Index>& freeDofs, size_t totalDofs) { return expandSolutionVector(freeSolution, freeDofs, totalDofs); }
    VectorXd solveWithLDLTPublic(const MatrixXd& K, const VectorXd& F) { return solveWithLDLT(K, F); }
    #endif

private:
    AnalysisOptions m_options;              ///< Analysis configuration
    AnalysisResults m_lastResults;          ///< Last analysis results
    
    // Helper methods for matrix assembly
    void assembleMemberStiffness(const Member& member, MatrixXd& globalK);
    void assembleMemberLoad(const Member& member, VectorXd& globalF);
    
    // DOF mapping utilities
    std::vector<Index> getFreeDofIndices(const Truss& truss);
    std::vector<Index> getConstrainedDofIndices(const Truss& truss);
    
    // Matrix utilities
    MatrixXd extractFreeStiffnessMatrix(const MatrixXd& K, const std::vector<Index>& freeDofs);
    VectorXd extractFreeLoadVector(const VectorXd& F, const std::vector<Index>& freeDofs);
    VectorXd expandSolutionVector(const VectorXd& freeSolution, 
                                  const std::vector<Index>& freeDofs, 
                                  size_t totalDofs);
    
    // Solver implementations
    VectorXd solveWithLU(const MatrixXd& K, const VectorXd& F);
    VectorXd solveWithLDLT(const MatrixXd& K, const VectorXd& F);
    VectorXd solveWithIterative(const MatrixXd& K, const VectorXd& F);
    
    // Analysis validation
    bool validateInputs(const Truss& truss);
    bool checkConvergence(const VectorXd& residual, Real tolerance);
    Real computeConditionNumber(const MatrixXd& K);
    
    // Post-processing utilities
    Real computeStrainEnergy(const Truss& truss, const VectorXd& displacements);
    Real findMaxDisplacement(const VectorXd& displacements);
    Real findMaxStress(const std::vector<Real>& stresses);
    
    // Logging and output
    void logAnalysisStart(const Truss& truss);
    void logAnalysisProgress(int iteration, Real residual);
    void logAnalysisComplete(const AnalysisResults& results);
};

// Type aliases
using AnalysisEnginePtr = std::shared_ptr<AnalysisEngine>;
using AnalysisEngineConstPtr = std::shared_ptr<const AnalysisEngine>;

} // namespace truss::core
