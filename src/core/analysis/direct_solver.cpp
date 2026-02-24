/**
 * @file direct_solver.cpp
 * @brief Direct linear equation solver using LU decomposition.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "direct_solver.hpp"

#include <Eigen/Cholesky>
#include <Eigen/LU>

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

    // Check if matrix is symmetric within a relative tolerance.
    // 1e-10 is chosen to handle floating-point assembly errors in stiffness
    // matrices while still detecting genuinely non-symmetric matrices.
    bool isSymmetric = A.isApprox(A.transpose(), 1e-10);

    if (isSymmetric) {
        // Use LDLT for symmetric positive definite matrices (more efficient)
        Eigen::LDLT<MatrixXd> ldlt(A);

        // Check if decomposition was successful
        if (ldlt.info() != Eigen::Success) {
            throw std::runtime_error("DirectSolver: LDLT decomposition failed - matrix may be "
                                     "singular or not positive definite");
        }

        // Solve the system
        VectorXd x = ldlt.solve(b);
        return x;
    } else {
        // Use FullPivLU for general non-symmetric matrices
        // This is more robust for ill-conditioned and rank-deficient matrices
        Eigen::FullPivLU<MatrixXd> lu(A);

        // Check if matrix is singular
        if (!lu.isInvertible()) {
            throw std::runtime_error("DirectSolver: Matrix is singular or rank-deficient");
        }

        // Solve the system
        VectorXd x = lu.solve(b);
        return x;
    }
}

}  // namespace truss::core::analysis
