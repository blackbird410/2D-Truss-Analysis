/**
 * @file IterativeSolver.hpp
 * @brief Iterative linear solver using Conjugate Gradient method
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "ilinear_solver.hpp"

namespace truss::core::analysis {

/**
 * @brief Iterative solver using Conjugate Gradient (CG) method
 *
 * Implements iterative solution of linear systems using Eigen's Conjugate
 * Gradient method, which is particularly efficient for large, sparse,
 * symmetric positive definite systems.
 *
 * Algorithm: Conjugate Gradient iteration
 * - Starts with an initial guess (typically zero vector)
 * - Iteratively refines the solution using gradient descent
 * - Converges to solution within specified tolerance
 *
 * Performance Characteristics:
 * - Complexity: O(n²) per iteration (for dense matrices)
 * - Memory: O(n) working memory (plus matrix storage)
 * - Accuracy: Controlled by tolerance parameter
 * - Best for: Large sparse systems (> 10,000 DOFs)
 *
 * Advantages:
 * - Memory efficient (no matrix factorization required)
 * - Faster than direct methods for large sparse systems
 * - Can use preconditioners for faster convergence
 * - Natural handling of very large systems
 *
 * Limitations:
 * - Requires symmetric positive definite matrix
 * - May not converge for ill-conditioned matrices
 * - Convergence rate depends on matrix condition number
 * - Solution is approximate (within tolerance)
 *
 * @note For small systems (< 1,000 DOFs), DirectSolver is usually faster
 * @note For large well-conditioned systems, this solver is preferred
 *
 * @see ILinearSolver Base interface
 * @see DirectSolver Alternative for small/medium systems
 */
class IterativeSolver : public ILinearSolver {
public:
    /**
     * @brief Construct iterative solver with custom parameters
     *
     * @param maxIterations Maximum number of CG iterations (default: 1000)
     * @param tolerance Convergence tolerance (default: 1e-9)
     *
     * @note Lower tolerance = more accurate but slower convergence
     * @note Higher maxIterations = more robust but potentially slower
     *
     * Recommended values:
     * - Standard analysis: maxIterations=1000, tolerance=1e-9
     * - Quick approximation: maxIterations=100, tolerance=1e-6
     * - High precision: maxIterations=10000, tolerance=1e-12
     */
    explicit IterativeSolver(int maxIterations = 1000, double tolerance = 1e-9);

    /**
     * @brief Destructor
     */
    ~IterativeSolver() override = default;

    /**
     * @brief Solve linear system using Conjugate Gradient method
     *
     * Iteratively solves the system Ax = b using the CG algorithm.
     * The method converges when ||Ax - b|| < tolerance * ||b||.
     *
     * @param A Coefficient matrix (must be symmetric positive definite)
     * @param b Right-hand side vector
     * @return Solution vector x such that Ax ≈ b (within tolerance)
     *
     * @throws std::runtime_error if:
     *   - Matrix is not positive definite
     *   - Solver fails to converge within maxIterations
     *   - Matrix dimensions are incompatible
     *
     * @note Convergence rate depends on condition number of A:
     *       - Well-conditioned: Fast convergence (< 100 iterations)
     *       - Ill-conditioned: Slow convergence (may not converge)
     *
     * @note For better performance with ill-conditioned systems,
     *       consider using a preconditioner (future enhancement)
     *
     * Example:
     * @code
     * IterativeSolver solver(1000, 1e-9);  // Max 1000 iterations, 1e-9 tolerance
     * MatrixXd K(10000, 10000);  // Large stiffness matrix
     * VectorXd F(10000);         // Load vector
     * // ... fill K and F ...
     *
     * VectorXd u = solver.solve(K, F);  // Solve Ku = F iteratively
     * @endcode
     */
    VectorXd solve(const MatrixXd& A, const VectorXd& b) const override;

    /**
     * @brief Get solver name
     *
     * @return "IterativeSolver (ConjugateGradient)"
     */
    std::string getName() const override { return "IterativeSolver (ConjugateGradient)"; }

    /**
     * @brief Get maximum number of iterations
     */
    [[maybe_unused]] int getMaxIterations() const noexcept { return m_maxIterations; }

    /**
     * @brief Get convergence tolerance
     */
    [[maybe_unused]] double getTolerance() const noexcept { return m_tolerance; }

private:
    int m_maxIterations;  ///< Maximum number of CG iterations
    double m_tolerance;   ///< Convergence tolerance
};

}  // namespace truss::core::analysis
