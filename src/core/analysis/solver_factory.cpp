/**
 * @file SolverFactory.cpp
 * @brief Implementation of SolverFactory class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "solver_factory.hpp"

namespace truss::core::analysis {

[[maybe_unused]] std::unique_ptr<ILinearSolver> SolverFactory::createSolver(SolverType type) {
    switch (type) {
        case SolverType::Direct:
            return std::make_unique<DirectSolver>();

        case SolverType::Iterative:
            return std::make_unique<IterativeSolver>();

        default:
            throw std::invalid_argument("SolverFactory: Unknown solver type");
    }
}

[[maybe_unused]] std::unique_ptr<ILinearSolver> SolverFactory::createDirectSolver() {
    return std::make_unique<DirectSolver>();
}

[[maybe_unused]] std::unique_ptr<ILinearSolver>
SolverFactory::createIterativeSolver(int maxIterations, double tolerance) {
    return std::make_unique<IterativeSolver>(maxIterations, tolerance);
}

}  // namespace truss::core::analysis
