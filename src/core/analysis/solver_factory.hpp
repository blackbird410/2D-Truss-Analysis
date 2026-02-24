/**
 * @file solver_factory.hpp
 * @brief Factory for creating linear solver instances.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include "direct_solver.hpp"
#include "ilinear_solver.hpp"
#include "iterative_solver.hpp"

#include <memory>

namespace truss::core::analysis {

/**
 * @brief Enumeration of available solver types
 */
enum class SolverType {
    Direct,    ///< Direct solver using LDLT factorization
    Iterative  ///< Iterative solver using Conjugate Gradient
};

/**
 * @brief Factory class for creating linear solver instances
 *
 * Provides static factory methods to create solver objects without
 * requiring client code to know about specific solver implementations.
 * This follows the Factory design pattern, enabling flexible solver
 * selection at runtime.
 *
 * Usage Patterns:
 * 1. Create solver with default parameters:
 *    ```cpp
 *    auto solver = SolverFactory::createSolver(SolverType::Direct);
 *    ```
 *
 * 2. Create solver with custom parameters:
 *    ```cpp
 *    auto solver = SolverFactory::createIterativeSolver(2000, 1e-10);
 *    ```
 *
 * 3. Select solver based on problem size:
 *    ```cpp
 *    SolverType type = (numDofs > 10000) ? SolverType::Iterative : SolverType::Direct;
 *    auto solver = SolverFactory::createSolver(type);
 *    ```
 *
 * Solver Selection Guidelines:
 * - Use DirectSolver when:
 *   • System size < 10,000 DOFs
 *   • Matrix is well-conditioned
 *   • Exact solution required
 *   • Multiple right-hand sides to solve
 *
 * - Use IterativeSolver when:
 *   • System size > 10,000 DOFs
 *   • Matrix is sparse
 *   • Approximate solution acceptable
 *   • Memory is limited
 *
 * @note All factory methods return unique_ptr for automatic memory management
 * @note Solvers are stateless and can be reused for multiple problems
 *
 * @see ILinearSolver Base solver interface
 * @see DirectSolver Direct solver implementation
 * @see IterativeSolver Iterative solver implementation
 */
class SolverFactory {
public:
    /**
     * @brief Create a linear solver of the specified type
     *
     * Factory method that creates a solver instance based on the
     * requested type, using default parameters for the solver.
     *
     * @param type Type of solver to create
     * @return Unique pointer to solver instance
     *
     * Example:
     * @code
     * auto directSolver = SolverFactory::createSolver(SolverType::Direct);
     * auto iterativeSolver = SolverFactory::createSolver(SolverType::Iterative);
     *
     * MatrixXd K(100, 100);
     * VectorXd F(100);
     * // ... fill K and F ...
     *
     * VectorXd u1 = directSolver->solve(K, F);
     * VectorXd u2 = iterativeSolver->solve(K, F);
     * @endcode
     */
    [[maybe_unused]] static std::unique_ptr<ILinearSolver> createSolver(SolverType type);

    /**
     * @brief Create a direct solver instance
     *
     * Convenience method for creating a DirectSolver with default parameters.
     * Equivalent to createSolver(SolverType::Direct).
     *
     * @return Unique pointer to DirectSolver instance
     *
     * Example:
     * @code
     * auto solver = SolverFactory::createDirectSolver();
     * VectorXd x = solver->solve(A, b);
     * @endcode
     */
    [[maybe_unused]] static std::unique_ptr<ILinearSolver> createDirectSolver();

    /**
     * @brief Create an iterative solver with custom parameters
     *
     * Creates an IterativeSolver with specified maximum iterations
     * and convergence tolerance.
     *
     * @param maxIterations Maximum number of CG iterations (default: 1000)
     * @param tolerance Convergence tolerance (default: 1e-9)
     * @return Unique pointer to IterativeSolver instance
     *
     * @throws std::invalid_argument if maxIterations <= 0 or tolerance <= 0
     *
     * Example:
     * @code
     * // High-precision solver
     * auto solver = SolverFactory::createIterativeSolver(10000, 1e-12);
     * VectorXd x = solver->solve(A, b);
     *
     * // Fast approximate solver
     * auto fastSolver = SolverFactory::createIterativeSolver(100, 1e-6);
     * VectorXd xApprox = fastSolver->solve(A, b);
     * @endcode
     */
    [[maybe_unused]] static std::unique_ptr<ILinearSolver>
    createIterativeSolver(int maxIterations = 1000, double tolerance = 1e-9);

private:
    // Private constructor to prevent instantiation
    SolverFactory() = delete;
};

}  // namespace truss::core::analysis
