#include "../../src/core/Truss.hpp"
#include "../../src/core/AnalysisEngine.hpp"
#include <iostream>
#include <iomanip>

using namespace truss::core;

int main() {
    std::cout << std::scientific << std::setprecision(3);
    
    // Create the same truss
    Truss truss("Free System Debug");
    
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    truss.assignDofNumbers();
    
    AnalysisEngine engine;
    
    std::cout << "=== DOF Analysis ===" << std::endl;
    auto freeDofs = engine.getFreeDofIndicesPublic(truss);
    auto constrainedDofs = engine.getConstrainedDofIndicesPublic(truss);
    
    std::cout << "Free DOFs: ";
    for (auto dof : freeDofs) {
        std::cout << dof << " ";
    }
    std::cout << " (count: " << freeDofs.size() << ")" << std::endl;
    
    std::cout << "Constrained DOFs: ";
    for (auto dof : constrainedDofs) {
        std::cout << dof << " ";
    }
    std::cout << " (count: " << constrainedDofs.size() << ")" << std::endl;
    
    // Get full system matrices
    MatrixXd K_full = engine.assembleStiffnessMatrix(truss);
    VectorXd F_full = engine.assembleLoadVector(truss);
    
    std::cout << "\n=== Full System ===" << std::endl;
    std::cout << "Full K determinant: " << K_full.determinant() << std::endl;
    std::cout << "Full K size: " << K_full.rows() << "x" << K_full.cols() << std::endl;
    
    // Extract free system
    MatrixXd K_free = engine.extractFreeStiffnessMatrixPublic(K_full, freeDofs);
    VectorXd F_free = engine.extractFreeLoadVectorPublic(F_full, freeDofs);
    
    std::cout << "\n=== Free System ===" << std::endl;
    std::cout << "Free K size: " << K_free.rows() << "x" << K_free.cols() << std::endl;
    std::cout << "Free K determinant: " << K_free.determinant() << std::endl;
    
    std::cout << "Free stiffness matrix K_free:" << std::endl;
    std::cout << K_free << std::endl;
    
    std::cout << "\nFree load vector F_free:" << std::endl;
    std::cout << F_free.transpose() << std::endl;
    
    // Try to solve the free system
    std::cout << "\n=== Solving Free System ===" << std::endl;
    try {
        VectorXd u_free = engine.solveWithLDLTPublic(K_free, F_free);
        std::cout << "Free solution u_free:" << std::endl;
        std::cout << u_free.transpose() << std::endl;
        
        // Expand to full solution
        VectorXd u_full = engine.expandSolutionVectorPublic(u_free, freeDofs, truss.getTotalDofs());
        std::cout << "\nFull solution u_full:" << std::endl;
        std::cout << u_full.transpose() << std::endl;
        
        std::cout << "\nMax displacement: " << u_full.cwiseAbs().maxCoeff() << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Failed to solve: " << e.what() << std::endl;
    }
    
    return 0;
}
