/**
 * @file AnalysisOrchestrator.cpp
 * @brief Implementation of the AnalysisOrchestrator class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "AnalysisOrchestrator.hpp"
#include "../Logger.hpp"
#include <Eigen/Eigenvalues>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace truss::core::analysis {

AnalysisOrchestrator::AnalysisOrchestrator(
    std::unique_ptr<ILinearSolver> solver,
    const AnalysisOptions& options)
    : m_assembler(std::make_unique<StiffnessAssembler>())
    , m_bcHandler(std::make_unique<BoundaryConditionHandler>())
    , m_solver(std::move(solver))
    , m_options(options) {
    
    if (!m_solver) {
        throw std::invalid_argument("Solver cannot be null");
    }
}

AnalysisResults AnalysisOrchestrator::analyze(Truss& truss) {
    logAnalysisStart(truss);
    
    // 1. Validate inputs
    if (!validateInputs(truss)) {
        throw std::runtime_error("Invalid truss structure for analysis");
    }
    
    // 2. Check structural validity
    if (m_options.checkStability && !checkStructuralValidity(truss)) {
        throw std::runtime_error("Truss structure is not stable or determinate");
    }
    
    // 3. Assign DOF numbers
    assignDOFs(truss);
    
    // 4. Assemble global stiffness matrix
    MatrixXd K = m_assembler->assemble(truss);
    
    // 5. Assemble load vector
    VectorXd F = assembleLoadVector(truss);
    
    // 6. Apply boundary conditions
    std::vector<Index> freeDofs = m_bcHandler->getFreeDofs(truss);
    MatrixXd Kff = m_bcHandler->applyToStiffness(K, freeDofs);
    VectorXd Ff = m_bcHandler->applyToLoad(F, freeDofs);
    
    // 7. Solve the reduced system
    VectorXd freeDisplacements = m_solver->solve(Kff, Ff);
    
    // 8. Expand solution to full displacement vector
    VectorXd displacements = m_bcHandler->expandDisplacements(freeDisplacements, freeDofs, truss.getTotalDofs());
    
    // 9. Post-process results
    AnalysisResults results = postProcessResults(truss, displacements, K);
    
    // Store results
    m_lastResults = results;
    
    // Update truss with results
    updateTrussResults(truss, results);
    
    logAnalysisComplete(results);
    
    return results;
}

void AnalysisOrchestrator::assignDOFs(Truss& truss) {
    truss.assignDofNumbers();
}

bool AnalysisOrchestrator::validateInputs(const Truss& truss) const {
    // Check minimum requirements
    if (truss.getNodes().empty() || truss.getMembers().empty()) {
        Logger::error("Truss must have at least one node and one member");
        return false;
    }
    
    // Check that all nodes have valid coordinates
    const auto& nodes = truss.getNodes();
    for (const auto& node : nodes) {
        const Point2D& pos = node->getPosition();
        if (!std::isfinite(pos.x) || !std::isfinite(pos.y)) {
            Logger::error("Node has invalid coordinates");
            return false;
        }
    }
    
    // Check that all members have valid properties
    const auto& members = truss.getMembers();
    for (const auto& member : members) {
        if (member->getLength() <= 0.0) {
            Logger::error("Member has zero or negative length");
            return false;
        }
        
        const auto& material = member->getMaterial();
        if (material.youngModulus <= 0.0) {
            Logger::error("Member has invalid Young's modulus");
            return false;
        }
        
        const auto& section = member->getSection();
        if (section.area <= 0.0) {
            Logger::error("Member has invalid cross-sectional area");
            return false;
        }
    }
    
    return true;
}

bool AnalysisOrchestrator::checkStructuralValidity(const Truss& truss) const {
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

VectorXd AnalysisOrchestrator::assembleLoadVector(const Truss& truss) const {
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
    
    return F;
}

std::vector<Real> AnalysisOrchestrator::computeMemberForces(
    const Truss& truss, 
    const VectorXd& displacements) const {
    
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
        
        // Get unit vector components
        Vector2d unitVec = member->getUnitVector();
        Real c = unitVec.x();
        Real s = unitVec.y();
        
        // Compute axial displacement
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

VectorXd AnalysisOrchestrator::computeReactions(
    const Truss& truss,
    const VectorXd& displacements,
    const MatrixXd& K) const {
    
    if (!m_options.computeReactions) {
        return VectorXd::Zero(truss.getConstrainedDofs());
    }
    
    // Compute all forces from stiffness: F = K * u
    VectorXd allForces = K * displacements;
    
    // Extract reaction forces at constrained DOFs
    std::vector<Index> constrainedDofs = m_bcHandler->getConstrainedDofs(truss);
    VectorXd reactions(constrainedDofs.size());
    
    for (size_t i = 0; i < constrainedDofs.size(); ++i) {
        reactions(i) = allForces(constrainedDofs[i]);
    }
    
    return reactions;
}

AnalysisResults AnalysisOrchestrator::postProcessResults(
    Truss& truss,
    const VectorXd& displacements,
    const MatrixXd& K) {
    
    AnalysisResults results;
    
    // Convert displacement vector to std::vector
    results.displacements.resize(displacements.size());
    for (int i = 0; i < displacements.size(); ++i) {
        results.displacements[i] = displacements(i);
    }
    
    // Compute member forces
    results.memberForces = computeMemberForces(truss, displacements);
    
    // Compute reactions
    VectorXd reactions = computeReactions(truss, displacements, K);
    results.reactions.resize(reactions.size());
    for (int i = 0; i < reactions.size(); ++i) {
        results.reactions[i] = reactions(i);
    }
    
    // Store stiffness matrix
    results.stiffnessMatrix.resize(K.rows());
    for (int i = 0; i < K.rows(); ++i) {
        results.stiffnessMatrix[i].resize(K.cols());
        for (int j = 0; j < K.cols(); ++j) {
            results.stiffnessMatrix[i][j] = K(i, j);
        }
    }
    
    // Compute member stresses and utilization ratios
    const auto& members = truss.getMembers();
    results.memberStresses.resize(members.size());
    results.utilizationRatios.resize(members.size());
    
    for (size_t i = 0; i < members.size(); ++i) {
        Real force = results.memberForces[i];
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
    
    return results;
}

Real AnalysisOrchestrator::computeConditionNumber(const MatrixXd& K) const {
    if (K.rows() == 0 || K.cols() == 0) {
        return 0.0;
    }
    
    try {
        // Use eigenvalue decomposition for symmetric matrices
        Eigen::SelfAdjointEigenSolver<MatrixXd> solver(K);
        if (solver.info() != Eigen::Success) {
            return 0.0;
        }
        
        VectorXd eigenvalues = solver.eigenvalues();
        Real maxEigenvalue = eigenvalues.cwiseAbs().maxCoeff();
        Real minEigenvalue = eigenvalues.cwiseAbs().minCoeff();
        
        if (minEigenvalue < 1e-12) {
            return std::numeric_limits<Real>::infinity();
        }
        
        return maxEigenvalue / minEigenvalue;
    } catch (...) {
        return 0.0;
    }
}

Real AnalysisOrchestrator::computeStrainEnergy(
    const Truss& truss, 
    const VectorXd& displacements) const {
    
    Real totalEnergy = 0.0;
    const auto& members = truss.getMembers();
    
    for (const auto& member : members) {
        // Get member DOF indices
        auto dofIndices = member->getGlobalDofIndices();
        
        // Extract member displacements
        VectorXd memberDisp(4);
        for (size_t i = 0; i < 4 && i < dofIndices.size(); ++i) {
            memberDisp(i) = displacements(dofIndices[i]);
        }
        
        // Get unit vector
        Vector2d unitVec = member->getUnitVector();
        Real c = unitVec.x();
        Real s = unitVec.y();
        
        // Compute axial displacement
        Real u1x = memberDisp(0);
        Real u1y = memberDisp(1);
        Real u2x = memberDisp(2);
        Real u2y = memberDisp(3);
        
        Real axialDisp = (u2x - u1x) * c + (u2y - u1y) * s;
        
        // Strain energy: U = (1/2) * k * u^2
        Real stiffness = member->getStiffness();
        totalEnergy += 0.5 * stiffness * axialDisp * axialDisp;
    }
    
    return totalEnergy;
}

Real AnalysisOrchestrator::findMaxDisplacement(const VectorXd& displacements) const {
    if (displacements.size() == 0) {
        return 0.0;
    }
    
    Real maxDisp = 0.0;
    for (int i = 0; i < displacements.size(); i += 2) {
        Real dx = displacements(i);
        Real dy = (i + 1 < displacements.size()) ? displacements(i + 1) : 0.0;
        Real magnitude = std::sqrt(dx * dx + dy * dy);
        maxDisp = std::max(maxDisp, magnitude);
    }
    
    return maxDisp;
}

Real AnalysisOrchestrator::findMaxStress(const std::vector<Real>& stresses) const {
    if (stresses.empty()) {
        return 0.0;
    }
    
    Real maxStress = 0.0;
    for (Real stress : stresses) {
        maxStress = std::max(maxStress, std::abs(stress));
    }
    
    return maxStress;
}

void AnalysisOrchestrator::updateTrussResults(
    Truss& truss, 
    const AnalysisResults& results) {
    
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
    
    // Update node results
    for (size_t i = 0; i < nodes.size(); ++i) {
        Index dofX = nodes[i]->getDofX();
        Index dofY = nodes[i]->getDofY();
        
        NodeResults nodeResult;
        nodeResult.displacement.x = results.displacements[dofX];
        nodeResult.displacement.y = results.displacements[dofY];
        
        // Find reaction force if node is constrained
        if (nodes[i]->isConstrained()) {
            // Reactions are stored in order of constrained DOFs
            // We need to match this node's constrained DOFs
            nodeResult.reaction.fx = 0.0;
            nodeResult.reaction.fy = 0.0;
            
            std::vector<Index> constrainedDofs = m_bcHandler->getConstrainedDofs(truss);
            size_t reactionIndex = 0;
            
            for (const auto& node : nodes) {
                if (node->getSupportType() != SupportType::Free) {
                    if (node->getDofX() < dofX || 
                        (node->getDofX() == dofX && node->getDofY() < dofY)) {
                        
                        // Count how many constrained DOFs come before this node
                        if (node->getSupportType() == SupportType::Pinned) {
                            reactionIndex += 2;
                        } else if (node->getSupportType() == SupportType::PinnedX ||
                                   node->getSupportType() == SupportType::RollerX ||
                                   node->getSupportType() == SupportType::PinnedY ||
                                   node->getSupportType() == SupportType::RollerY) {
                            reactionIndex += 1;
                        }
                    }
                }
            }
            
            // Extract reactions for this node
            if (nodes[i]->getSupportType() == SupportType::Pinned) {
                if (reactionIndex + 1 < results.reactions.size()) {
                    nodeResult.reaction.fx = results.reactions[reactionIndex];
                    nodeResult.reaction.fy = results.reactions[reactionIndex + 1];
                }
            } else if (nodes[i]->getSupportType() == SupportType::PinnedX) {
                if (reactionIndex < results.reactions.size()) {
                    nodeResult.reaction.fx = results.reactions[reactionIndex];
                }
            } else if (nodes[i]->getSupportType() == SupportType::PinnedY) {
                if (reactionIndex < results.reactions.size()) {
                    nodeResult.reaction.fy = results.reactions[reactionIndex];
                }
            }
        }
        
        nodes[i]->setResults(nodeResult);
    }
}

void AnalysisOrchestrator::logAnalysisStart(const Truss& truss) const {
    if (!m_options.verbose) return;
    
    Logger::info("Starting structural analysis...");
    Logger::info("  Nodes: " + std::to_string(truss.getNodes().size()));
    Logger::info("  Members: " + std::to_string(truss.getMembers().size()));
    Logger::info("  Total DOFs: " + std::to_string(truss.getTotalDofs()));
    Logger::info("  Solver: " + m_solver->getName());
}

void AnalysisOrchestrator::logAnalysisComplete(const AnalysisResults& results) const {
    if (!m_options.verbose) return;
    
    Logger::info("Analysis complete!");
    Logger::info("  Converged: " + std::string(results.converged ? "Yes" : "No"));
    Logger::info("  Max displacement: " + std::to_string(results.maxDisplacement));
    Logger::info("  Max stress: " + std::to_string(results.maxStress));
}

} // namespace truss::core::analysis
