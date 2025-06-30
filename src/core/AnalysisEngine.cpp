/**
 * @file AnalysisEngine.cpp
 * @brief Implementation of the AnalysisEngine class
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
 */

#include "AnalysisEngine.hpp"
#include "Logger.hpp"
#include <Eigen/LU>
#include <Eigen/Cholesky>
#include <Eigen/IterativeLinearSolvers>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace truss::core {

AnalysisEngine::AnalysisEngine(const AnalysisOptions& options)
    : m_options(options) {
}

AnalysisResults AnalysisEngine::analyze(Truss& truss) {
    logAnalysisStart(truss);
    
    // Validate inputs
    if (!validateInputs(truss)) {
        throw std::runtime_error("Invalid truss structure for analysis");
    }
    
    // Check structural validity
    if (m_options.checkStability && !checkStructuralValidity(truss)) {
        throw std::runtime_error("Truss structure is not stable or determinate");
    }
    
    // Ensure DOF numbers are assigned
    truss.assignDofNumbers();
    
    // Assemble system matrices
    MatrixXd K = assembleStiffnessMatrix(truss);
    VectorXd F = assembleLoadVector(truss);
    
    // Solve the system
    VectorXd displacements = solveSystem(K, F, truss);
    
    // Compute derived quantities
    std::vector<Real> memberForces = computeMemberForces(truss, displacements);
    VectorXd reactions = computeReactions(truss, displacements);
    
    // Prepare results
    AnalysisResults results;
    results.displacements = displacements;
    results.reactions = reactions;
    results.memberForces = memberForces;
    
    // Compute member stresses and utilization ratios
    const auto& members = truss.getMembers();
    results.memberStresses.resize(members.size());
    results.utilizationRatios.resize(members.size());
    
    for (size_t i = 0; i < members.size(); ++i) {
        Real force = memberForces[i];
        Real area = members[i]->getSection().area;
        Real stress = force / area;
        Real yieldStrength = members[i]->getMaterial().yieldStrength;
        Real utilization = std::abs(stress) / yieldStrength;
        
        results.memberStresses[i] = stress;
        results.utilizationRatios[i] = utilization;
    }
    
    // Fill in metadata
    results.converged = true;
    results.totalDofs = truss.getTotalDofs();
    results.freeDofs = truss.getFreeDofs();
    results.constrainedDofs = truss.getConstrainedDofs();
    results.conditionNumber = computeConditionNumber(K);
    results.totalStrain = computeStrainEnergy(truss, displacements);
    results.maxDisplacement = findMaxDisplacement(displacements);
    results.maxStress = findMaxStress(results.memberStresses);
    
    // Update truss with results
    updateTrussResults(truss, results);
    
    // Store results
    m_lastResults = results;
    
    logAnalysisComplete(results);
    
    return results;
}

MatrixXd AnalysisEngine::assembleStiffnessMatrix(const Truss& truss) {
    size_t totalDofs = truss.getTotalDofs();
    MatrixXd K = MatrixXd::Zero(totalDofs, totalDofs);
    
    const auto& members = truss.getMembers();
    for (const auto& member : members) {
        assembleMemberStiffness(*member, K);
    }
    
    return K;
}

VectorXd AnalysisEngine::assembleLoadVector(const Truss& truss) {
    size_t totalDofs = truss.getTotalDofs();
    VectorXd F = VectorXd::Zero(totalDofs);
    
    // Assemble nodal forces
    const auto& nodes = truss.getNodes();
    for (const auto& node : nodes) {
        Index dofX = node->getDofX();
        Index dofY = node->getDofY();
        Force2D appliedForce = node->getAppliedForce();
        
        F(dofX) += appliedForce.fx;
        F(dofY) += appliedForce.fy;
    }
    
    // Add member self-weight (if implemented)
    const auto& members = truss.getMembers();
    for (const auto& member : members) {
        assembleMemberLoad(*member, F);
    }
    
    return F;
}

VectorXd AnalysisEngine::solveSystem(const MatrixXd& K, const VectorXd& F, const Truss& truss) {
    // Get free DOF indices
    std::vector<Index> freeDofs = getFreeDofIndices(truss);
    
    // Extract free system
    MatrixXd Kff = extractFreeStiffnessMatrix(K, freeDofs);
    VectorXd Ff = extractFreeLoadVector(F, freeDofs);
    
    // Solve free system
    VectorXd freeSolution;
    if (m_options.useDirectSolver) {
        freeSolution = solveWithLDLT(Kff, Ff);
    } else {
        freeSolution = solveWithIterative(Kff, Ff);
    }
    
    // Expand to full solution vector
    return expandSolutionVector(freeSolution, freeDofs, truss.getTotalDofs());
}

std::vector<Real> AnalysisEngine::computeMemberForces(const Truss& truss, const VectorXd& displacements) {
    const auto& members = truss.getMembers();
    std::vector<Real> forces;
    forces.reserve(members.size());
    
    for (const auto& member : members) {
        // Get member DOF indices
        auto dofIndices = member->getGlobalDofIndices();
        
        // Extract member displacements
        VectorXd memberDisp(4);
        for (size_t i = 0; i < 4 && i < dofIndices.size(); ++i) {
            memberDisp(i) = displacements(dofIndices[i]);
        }
        
        // Get local stiffness matrix
        MatrixXd Klocal = member->getLocalStiffnessMatrix();
        
        // Get unit vector components
        Vector2d unitVec = member->getUnitVector();
        Real c = unitVec.x();
        Real s = unitVec.y();
        
        // Compute axial displacement directly
        Real u1x = memberDisp(0);
        Real u1y = memberDisp(1);
        Real u2x = memberDisp(2);
        Real u2y = memberDisp(3);
        
        Real axialDisp = (u2x - u1x) * c + (u2y - u1y) * s;
        
        // Compute axial force
        Real axialForce = member->getStiffness() * axialDisp;
        forces.push_back(axialForce);
    }
    
    return forces;
}

VectorXd AnalysisEngine::computeReactions(const Truss& truss, const VectorXd& displacements) {
    if (!m_options.computeReactions) {
        return VectorXd::Zero(truss.getConstrainedDofs());
    }
    
    // Assemble full stiffness matrix and multiply by displacements
    MatrixXd K = assembleStiffnessMatrix(truss);
    VectorXd allForces = K * displacements;
    
    // Extract reaction forces at constrained DOFs
    std::vector<Index> constrainedDofs = getConstrainedDofIndices(truss);
    VectorXd reactions(constrainedDofs.size());
    
    for (size_t i = 0; i < constrainedDofs.size(); ++i) {
        reactions(i) = allForces(constrainedDofs[i]);
    }
    
    return reactions;
}

void AnalysisEngine::updateTrussResults(Truss& truss, const AnalysisResults& results) {
    const auto& members = truss.getMembers();
    const auto& nodes = truss.getNodes();
    
    // Update member results
    for (size_t i = 0; i < members.size(); ++i) {
        MemberResults memberResult;
        memberResult.axialForce = results.memberForces[i];
        memberResult.axialStress = results.memberStresses[i];
        memberResult.utilizationRatio = results.utilizationRatios[i];
        memberResult.inTension = results.memberForces[i] > 0;
        memberResult.yielded = results.utilizationRatios[i] > 1.0;
        
        members[i]->setResults(memberResult);
    }
    
    // Update node results with displacements and reactions
    for (size_t i = 0; i < nodes.size(); ++i) {
        Index dofX = nodes[i]->getDofX();
        Index dofY = nodes[i]->getDofY();
        
        NodeResults nodeResult;
        nodeResult.displacement.x = results.displacements(dofX);
        nodeResult.displacement.y = results.displacements(dofY);
        
        // Set reactions for constrained nodes
        std::vector<Index> constrainedDofs = getConstrainedDofIndices(truss);
        
        // Find if this node's DOFs are constrained and get their reactions
        Real reactionX = 0.0, reactionY = 0.0;
        SupportType support = nodes[i]->getSupportType();
        
        if (support != SupportType::Free) {
            // Check if X DOF is constrained
            if (support == SupportType::Pinned || support == SupportType::PinnedX) {
                auto it = std::find(constrainedDofs.begin(), constrainedDofs.end(), dofX);
                if (it != constrainedDofs.end()) {
                    size_t reactionIndex = std::distance(constrainedDofs.begin(), it);
                    reactionX = results.reactions(reactionIndex);
                }
            }
            
            // Check if Y DOF is constrained
            if (support == SupportType::Pinned || support == SupportType::PinnedY || 
                support == SupportType::RollerX || support == SupportType::RollerY) {
                auto it = std::find(constrainedDofs.begin(), constrainedDofs.end(), dofY);
                if (it != constrainedDofs.end()) {
                    size_t reactionIndex = std::distance(constrainedDofs.begin(), it);
                    reactionY = results.reactions(reactionIndex);
                }
            }
        }
        
        nodeResult.reaction = Force2D{reactionX, reactionY};
        
        nodes[i]->setResults(nodeResult);
    }
}

bool AnalysisEngine::checkStructuralValidity(const Truss& truss) {
    // Check basic validity
    if (!truss.isValid()) {
        return false;
    }
    
    // Check static determinacy
    if (!truss.isStaticallyDeterminate()) {
        if (m_options.verbose) {
            Logger::warn("Truss is not statically determinate");
        }
        return false;
    }
    
    // Check kinematic stability
    if (!truss.isKinematicallyStable()) {
        if (m_options.verbose) {
            Logger::warn("Truss is not kinematically stable");
        }
        return false;
    }
    
    return true;
}

// Private helper methods

void AnalysisEngine::assembleMemberStiffness(const Member& member, MatrixXd& globalK) {
    auto dofIndices = member.getGlobalDofIndices();
    MatrixXd memberK = member.getGlobalStiffnessMatrix();
    
    // Add member stiffness to global matrix
    for (size_t i = 0; i < dofIndices.size(); ++i) {
        for (size_t j = 0; j < dofIndices.size(); ++j) {
            globalK(dofIndices[i], dofIndices[j]) += memberK(i, j);
        }
    }
}

void AnalysisEngine::assembleMemberLoad(const Member& member, VectorXd& globalF) {
    // For now, we don't consider member self-weight
    // This could be extended to include distributed loads
    (void)member;  // Suppress unused parameter warning
    (void)globalF;
}

std::vector<Index> AnalysisEngine::getFreeDofIndices(const Truss& truss) {
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
            freeDofs.push_back(dofX); // X DOF
        }
        
        if (support == SupportType::Free || 
            support == SupportType::PinnedX || 
            support == SupportType::RollerY) {
            freeDofs.push_back(dofY); // Y DOF
        }
    }
    
    // Sort the indices
    std::sort(freeDofs.begin(), freeDofs.end());
    
    return freeDofs;
}

std::vector<Index> AnalysisEngine::getConstrainedDofIndices(const Truss& truss) {
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
            constrainedDofs.push_back(dofX); // X DOF
        }
        
        if (support == SupportType::Pinned || 
            support == SupportType::PinnedY || 
            support == SupportType::RollerX) {
            constrainedDofs.push_back(dofY); // Y DOF
        }
    }
    
    // Sort the indices
    std::sort(constrainedDofs.begin(), constrainedDofs.end());
    
    return constrainedDofs;
}

MatrixXd AnalysisEngine::extractFreeStiffnessMatrix(const MatrixXd& K, const std::vector<Index>& freeDofs) {
    size_t n = freeDofs.size();
    MatrixXd Kff(n, n);
    
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            Kff(i, j) = K(freeDofs[i], freeDofs[j]);
        }
    }
    
    return Kff;
}

VectorXd AnalysisEngine::extractFreeLoadVector(const VectorXd& F, const std::vector<Index>& freeDofs) {
    size_t n = freeDofs.size();
    VectorXd Ff(n);
    
    for (size_t i = 0; i < n; ++i) {
        Ff(i) = F(freeDofs[i]);
    }
    
    return Ff;
}

VectorXd AnalysisEngine::expandSolutionVector(const VectorXd& freeSolution, 
                                              const std::vector<Index>& freeDofs, 
                                              size_t totalDofs) {
    VectorXd fullSolution = VectorXd::Zero(totalDofs);
    
    for (size_t i = 0; i < freeDofs.size(); ++i) {
        fullSolution(freeDofs[i]) = freeSolution(i);
    }
    
    return fullSolution;
}

VectorXd AnalysisEngine::solveWithLU(const MatrixXd& K, const VectorXd& F) {
    Eigen::FullPivLU<MatrixXd> lu(K);
    if (!lu.isInvertible()) {
        throw std::runtime_error("Stiffness matrix is singular");
    }
    return lu.solve(F);
}

VectorXd AnalysisEngine::solveWithLDLT(const MatrixXd& K, const VectorXd& F) {
    Eigen::LDLT<MatrixXd> ldlt(K);
    if (ldlt.info() != Eigen::Success) {
        throw std::runtime_error("LDLT decomposition failed");
    }
    return ldlt.solve(F);
}

VectorXd AnalysisEngine::solveWithIterative(const MatrixXd& K, const VectorXd& F) {
    Eigen::ConjugateGradient<MatrixXd> cg;
    cg.setMaxIterations(m_options.maxIterations);
    cg.setTolerance(m_options.convergenceTolerance);
    cg.compute(K);
    
    VectorXd solution = cg.solve(F);
    
    if (cg.info() != Eigen::Success) {
        throw std::runtime_error("Iterative solver failed to converge");
    }
    
    return solution;
}

bool AnalysisEngine::validateInputs(const Truss& truss) {
    // Check if truss has nodes and members
    if (truss.getNodeCount() == 0 || truss.getMemberCount() == 0) {
        return false;
    }
    
    // Check if there are applied forces
    if (!truss.hasAppliedForces()) {
        Logger::warn("No applied forces found in truss");
    }
    
    // Check for basic structural validity
    return truss.isValid();
}

bool AnalysisEngine::checkConvergence(const VectorXd& residual, Real tolerance) {
    return residual.norm() < tolerance;
}

Real AnalysisEngine::computeConditionNumber(const MatrixXd& K) {
    Eigen::JacobiSVD<MatrixXd> svd(K);
    auto singularValues = svd.singularValues();
    return singularValues(0) / singularValues(singularValues.size() - 1);
}

Real AnalysisEngine::computeStrainEnergy(const Truss& truss, const VectorXd& displacements) {
    MatrixXd K = assembleStiffnessMatrix(truss);
    return 0.5 * displacements.transpose() * K * displacements;
}

Real AnalysisEngine::findMaxDisplacement(const VectorXd& displacements) {
    return displacements.cwiseAbs().maxCoeff();
}

Real AnalysisEngine::findMaxStress(const std::vector<Real>& stresses) {
    if (stresses.empty()) return 0.0;
    
    Real maxStress = 0.0;
    for (Real stress : stresses) {
        maxStress = std::max(maxStress, std::abs(stress));
    }
    return maxStress;
}

void AnalysisEngine::logAnalysisStart(const Truss& truss) {
    if (m_options.verbose) {
        Logger::info("Starting structural analysis");
        Logger::info("Nodes: " + std::to_string(truss.getNodeCount()));
        Logger::info("Members: " + std::to_string(truss.getMemberCount()));
        Logger::info("Total DOFs: " + std::to_string(truss.getTotalDofs()));
        Logger::info("Free DOFs: " + std::to_string(truss.getFreeDofs()));
    }
}

void AnalysisEngine::logAnalysisProgress(int iteration, Real residual) {
    if (m_options.verbose) {
        Logger::info("Iteration " + std::to_string(iteration) + 
                    ", Residual: " + std::to_string(residual));
    }
}

void AnalysisEngine::logAnalysisComplete(const AnalysisResults& results) {
    if (m_options.verbose) {
        Logger::info("Analysis completed successfully");
        Logger::info("Max displacement: " + std::to_string(results.maxDisplacement));
        Logger::info("Max stress: " + std::to_string(results.maxStress));
        Logger::info("Condition number: " + std::to_string(results.conditionNumber));
    }
}

} // namespace truss::core
