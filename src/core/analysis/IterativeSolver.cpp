/**
 * @file IterativeSolver.cpp
 * @brief Implementation of IterativeSolver class
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
 */

#include "IterativeSolver.hpp"
#include <Eigen/IterativeLinearSolvers>
#include <stdexcept>

namespace truss::core::analysis {

IterativeSolver::IterativeSolver(int maxIterations, double tolerance)
    : m_maxIterations(maxIterations)
    , m_tolerance(tolerance) {
    
    if (maxIterations <= 0) {
        throw std::invalid_argument("IterativeSolver: maxIterations must be positive");
    }
    if (tolerance <= 0.0) {
        throw std::invalid_argument("IterativeSolver: tolerance must be positive");
    }
}

VectorXd IterativeSolver::solve(const MatrixXd& A, const VectorXd& b) const {
    // Validate dimensions
    if (A.rows() != A.cols()) {
        throw std::runtime_error("IterativeSolver: Matrix A must be square");
    }
    if (A.rows() != b.size()) {
        throw std::runtime_error("IterativeSolver: Matrix dimensions incompatible with vector");
    }

    // Create conjugate gradient solver
    Eigen::ConjugateGradient<MatrixXd> cg;
    cg.setMaxIterations(m_maxIterations);
    cg.setTolerance(m_tolerance);
    
    // Compute matrix decomposition
    cg.compute(A);
    
    if (cg.info() != Eigen::Success) {
        throw std::runtime_error("IterativeSolver: Failed to compute matrix - may not be positive definite");
    }
    
    // Solve the system
    VectorXd x = cg.solve(b);
    
    // Check convergence
    if (cg.info() != Eigen::Success) {
        throw std::runtime_error("IterativeSolver: Failed to converge within " + 
                                 std::to_string(m_maxIterations) + " iterations");
    }
    
    return x;
}

} // namespace truss::core::analysis
