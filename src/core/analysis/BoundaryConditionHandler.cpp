/**
 * @file BoundaryConditionHandler.cpp
 * @brief Implementation of boundary condition application
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "BoundaryConditionHandler.hpp"
#include <algorithm>

namespace truss::core::analysis {

std::vector<Index> BoundaryConditionHandler::getFreeDofs(const Truss& truss) const {
    std::vector<Index> freeDofs;
    const auto& nodes = truss.getNodes();
    
    for (const auto& node : nodes) {
        Index dofX = node->getDofX();
        Index dofY = node->getDofY();
        SupportType support = node->getSupportType();
        
        // Add free DOFs based on support type
        if (support == SupportType::Free || 
            support == SupportType::PinnedY || 
            support == SupportType::RollerX) {
            freeDofs.push_back(dofX); // X DOF is free
        }
        
        if (support == SupportType::Free || 
            support == SupportType::PinnedX || 
            support == SupportType::RollerY) {
            freeDofs.push_back(dofY); // Y DOF is free
        }
    }
    
    // Sort the indices for consistent ordering
    std::sort(freeDofs.begin(), freeDofs.end());
    
    return freeDofs;
}

std::vector<Index> BoundaryConditionHandler::getConstrainedDofs(const Truss& truss) const {
    std::vector<Index> constrainedDofs;
    const auto& nodes = truss.getNodes();
    
    for (const auto& node : nodes) {
        Index dofX = node->getDofX();
        Index dofY = node->getDofY();
        SupportType support = node->getSupportType();
        
        // Add constrained DOFs based on support type
        if (support == SupportType::Pinned || 
            support == SupportType::PinnedX || 
            support == SupportType::RollerY) {
            constrainedDofs.push_back(dofX); // X DOF is constrained
        }
        
        if (support == SupportType::Pinned || 
            support == SupportType::PinnedY || 
            support == SupportType::RollerX) {
            constrainedDofs.push_back(dofY); // Y DOF is constrained
        }
    }
    
    // Sort the indices for consistent ordering
    std::sort(constrainedDofs.begin(), constrainedDofs.end());
    
    return constrainedDofs;
}

MatrixXd BoundaryConditionHandler::applyToStiffness(const MatrixXd& K, 
                                                     const std::vector<Index>& freeDofs) const {
    size_t n = freeDofs.size();
    MatrixXd Kff(n, n);
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            Kff(i, j) = K(freeDofs[i], freeDofs[j]);
        }
    }
    
    return Kff;
}

VectorXd BoundaryConditionHandler::applyToLoad(const VectorXd& F, 
                                                const std::vector<Index>& freeDofs) const {
    size_t n = freeDofs.size();
    VectorXd Ff(n);
    
    for (size_t i = 0; i < n; ++i) {
        Ff(i) = F(freeDofs[i]);
    }
    
    return Ff;
}

VectorXd BoundaryConditionHandler::expandDisplacements(const VectorXd& freeSolution,
                                                        const std::vector<Index>& freeDofs,
                                                        size_t totalDofs) const {
    VectorXd fullSolution = VectorXd::Zero(totalDofs);
    
    for (size_t i = 0; i < freeDofs.size(); ++i) {
        fullSolution(freeDofs[i]) = freeSolution(i);
    }
    
    return fullSolution;
}

} // namespace truss::core::analysis
