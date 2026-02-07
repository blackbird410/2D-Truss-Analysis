/**
 * @file DirectSolver.hpp
 * @brief Direct linear solver using LDLT factorization
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "ILinearSolver.hpp"

namespace truss::core::analysis {

/**
 * @brief Direct solver using LDLT matrix factorization
 * 
 * Implements direct solution of linear systems using Eigen's LDLT decomposition,
 * which is particularly efficient for symmetric positive definite matrices
 * (typical of stiffness matrices in structural analysis).
 * 
 * Algorithm: A = L * D * L^T, where:
 * - L is a unit lower triangular matrix
 * - D is a diagonal matrix
 * - L^T is the transpose of L
 * 
 * Performance Characteristics:
 * - Complexity: O(n³) for decomposition, O(n²) for back-substitution
 * - Memory: O(n²) for matrix storage
 * - Accuracy: Very good for well-conditioned matrices
 * - Best for: Small to medium systems (< 10,000 DOFs)
 * 
 * Advantages:
 * - Exact solution (within numerical precision)
 * - No convergence issues
 * - Robust for well-conditioned matrices
 * - Exploits symmetry of stiffness matrices
 * 
 * Limitations:
 * - Memory intensive for large systems
 * - Slower than iterative methods for very large sparse systems
 * - Fails for singular or near-singular matrices
 * 
 * @note This solver is the default choice for most truss analysis problems
 * @note For very large systems (> 10,000 DOFs), consider IterativeSolver
 * 
 * @see ILinearSolver Base interface
 * @see IterativeSolver Alternative for large systems
 */
class DirectSolver : public ILinearSolver {
public:
    /**
     * @brief Default constructor
     */
    DirectSolver() = default;

    /**
     * @brief Destructor
     */
    ~DirectSolver() override = default;

    /**
     * @brief Solve linear system using LDLT factorization
     * 
     * Decomposes the matrix A using LDLT factorization and solves the system.
     * This method is particularly efficient for symmetric positive definite
     * matrices, which are characteristic of structural stiffness matrices.
     * 
     * @param A Coefficient matrix (should be symmetric positive definite)
     * @param b Right-hand side vector
     * @return Solution vector x such that Ax = b
     * 
     * @throws std::runtime_error if:
     *   - LDLT decomposition fails (matrix not positive definite)
     *   - Matrix is singular or near-singular
     *   - Matrix dimensions are incompatible
     * 
     * @note For symmetric positive definite matrices, LDLT is:
     *       - More efficient than LU (requires half the storage)
     *       - Numerically stable
     *       - Exploits matrix structure
     * 
     * Example:
     * @code
     * DirectSolver solver;
     * MatrixXd K(100, 100);  // Stiffness matrix
     * VectorXd F(100);       // Load vector
     * // ... fill K and F ...
     * 
     * VectorXd u = solver.solve(K, F);  // Solve Ku = F
     * @endcode
     */
    VectorXd solve(const MatrixXd& A, const VectorXd& b) const override;

    /**
     * @brief Get solver name
     * 
     * @return "DirectSolver (LDLT)"
     */
    std::string getName() const override { return "DirectSolver (LDLT)"; }
};

} // namespace truss::core::analysis
