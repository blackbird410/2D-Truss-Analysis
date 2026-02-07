/**
 * @file DirectSolver.cpp
 * @brief Implementation of DirectSolver class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "DirectSolver.hpp"
#include <Eigen/Cholesky>
#include <stdexcept>

namespace truss::core::analysis {

VectorXd DirectSolver::solve(const MatrixXd& A, const VectorXd& b) const {
    // Validate dimensions
    if (A.rows() != A.cols()) {
        throw std::runtime_error("DirectSolver: Matrix A must be square");
    }
    if (A.rows() != b.size()) {
        throw std::runtime_error("DirectSolver: Matrix dimensions incompatible with vector");
    }

    // Perform LDLT decomposition
    Eigen::LDLT<MatrixXd> ldlt(A);
    
    // Check if decomposition was successful
    if (ldlt.info() != Eigen::Success) {
        throw std::runtime_error("DirectSolver: LDLT decomposition failed - matrix may be singular or not positive definite");
    }
    
    // Solve the system
    VectorXd x = ldlt.solve(b);
    
    return x;
}

} // namespace truss::core::analysis
