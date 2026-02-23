/**
 * @file analysis_options.hpp
 * @brief Configuration options for structural analysis
 */

#pragma once

#include <string>

namespace truss {

/**
 * @enum SolverType
 * @brief Available solver algorithms for linear system solution
 */
enum class SolverType {
    Direct,       ///< Direct solver (Gaussian elimination)
    Iterative     ///< Iterative solver (Conjugate gradient, etc.)
};

/**
 * @struct AnalysisOptions
 * @brief Configuration for structural analysis parameters
 * 
 * Controls:
 * - Which solver to use
 * - Convergence tolerance
 * - Maximum iterations
 * - Output verbosity
 */
struct AnalysisOptions {
    /// Type of solver to use
    SolverType solverType = SolverType::Direct;
    
    /// Convergence tolerance for iterative solvers
    double tolerance = 1e-8;
    
    /// Maximum iterations for iterative solvers
    int maxIterations = 10000;
    
    /// Enable verbose output
    bool verbose = false;
};

} // namespace truss
