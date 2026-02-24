/**
 * @file ilinear_solver.hpp
 * @brief Interface for linear system solvers.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include <Eigen/Dense>

#include <memory>

namespace truss::core::analysis {

using Eigen::MatrixXd;
using Eigen::VectorXd;

/**
 * @brief Abstract interface for linear system solvers
 *
 * Defines the contract for solving linear systems of the form Ax = b,
 * where A is the coefficient matrix and b is the right-hand side vector.
 *
 * This interface supports both direct and iterative solution methods,
 * allowing for flexible solver selection based on problem characteristics:
 * - Small to medium systems (< 10,000 DOFs): Direct solvers preferred
 * - Large systems (> 10,000 DOFs): Iterative solvers may be more efficient
 * - Ill-conditioned systems: Direct solvers more robust
 *
 * @note All implementations must handle singular/ill-conditioned matrices
 *       by throwing std::runtime_error with descriptive messages
 *
 * @see DirectSolver Direct solution using matrix factorization
 * @see IterativeSolver Iterative solution using conjugate gradient method
 * @see SolverFactory Factory for creating solver instances
 */
class ILinearSolver {
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes
     */
    virtual ~ILinearSolver() = default;

    /**
     * @brief Solve the linear system Ax = b
     *
     * Computes the solution vector x for the linear system Ax = b using
     * the specific solution method implemented by the derived class.
     *
     * @param A Coefficient matrix (must be square and non-singular)
     * @param b Right-hand side vector
     * @return Solution vector x
     *
     * @throws std::runtime_error if:
     *   - Matrix A is singular or nearly singular
     *   - Matrix dimensions are incompatible (A.rows() != b.size())
     *   - Solver fails to converge (iterative solvers)
     *   - Numerical issues prevent solution
     *
     * @pre A must be square (A.rows() == A.cols())
     * @pre A.rows() must equal b.size()
     * @pre Matrix A should be well-conditioned for accurate results
     *
     * @post Return value has size equal to A.cols() == b.size()
     * @post If successful, A*x ≈ b within numerical tolerance
     *
     * Example:
     * @code
     * MatrixXd K(10, 10);  // Stiffness matrix
     * VectorXd F(10);      // Load vector
     * // ... fill K and F ...
     *
     * std::unique_ptr<ILinearSolver> solver = SolverFactory::createDirectSolver();
     * VectorXd displacements = solver->solve(K, F);
     * @endcode
     */
    virtual VectorXd solve(const MatrixXd& A, const VectorXd& b) const = 0;

    /**
     * @brief Get the solver type name.
     *
     * Returns a human-readable identifier for the solver type,
     * useful for logging and debugging purposes.
     *
     * @return Solver name (e.g., "DirectSolver", "IterativeSolver")
     */
    virtual std::string getName() const = 0;
};

}  // namespace truss::core::analysis
