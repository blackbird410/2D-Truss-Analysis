#include "../../src/core/Truss.hpp"
#include "../../src/core/AnalysisEngine.hpp"
#include <iostream>
#include <iomanip>

using namespace truss::core;

// Helper function to access private methods via reflection 
class DebugAnalysisEngine : public AnalysisEngine {
public:
    std::vector<Index> getFreeDofIndicesPublic(const Truss& truss) {
        return getFreeDofIndices(truss);
    }
    
    std::vector<Index> getConstrainedDofIndicesPublic(const Truss& truss) {
        return getConstrainedDofIndices(truss);
    }
    
    MatrixXd extractFreeStiffnessMatrixPublic(const MatrixXd& K, const std::vector<Index>& freeDofs) {
        return extractFreeStiffnessMatrix(K, freeDofs);
    }
    
    VectorXd extractFreeLoadVectorPublic(const VectorXd& F, const std::vector<Index>& freeDofs) {
        return extractFreeLoadVector(F, freeDofs);
    }
};

int main() {
    std::cout << std::scientific << std::setprecision(3);
    
    // Create the same truss
    Truss truss("Constraint Debug");
    
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY);
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    truss.addMember(node1, node2);
    truss.addMember(node1, node3);
    truss.addMember(node2, node3);
    
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    truss.assignDofNumbers();
    
    // Debug constraint handling
    DebugAnalysisEngine engine;
    
    std::cout << "Node constraints:" << std::endl;
    std::cout << "Node 1 (Pinned): X=" << node1->getDofX() << ", Y=" << node1->getDofY() << std::endl;
    std::cout << "Node 2 (RollerY): X=" << node2->getDofX() << ", Y=" << node2->getDofY() << std::endl;
    std::cout << "Node 3 (Free): X=" << node3->getDofX() << ", Y=" << node3->getDofY() << std::endl;
    
    auto freeDofs = engine.getFreeDofIndicesPublic(truss);
    auto constrainedDofs = engine.getConstrainedDofIndicesPublic(truss);
    
    std::cout << "\nFree DOFs: ";
    for (auto dof : freeDofs) {
        std::cout << dof << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Constrained DOFs: ";
    for (auto dof : constrainedDofs) {
        std::cout << dof << " ";
    }
    std::cout << std::endl;
    
    // Get matrices
    MatrixXd K = engine.assembleStiffnessMatrix(truss);
    VectorXd F = engine.assembleLoadVector(truss);
    
    std::cout << "\nFull stiffness matrix determinant: " << K.determinant() << std::endl;
    
    // Extract free system
    MatrixXd Kff = engine.extractFreeStiffnessMatrixPublic(K, freeDofs);
    VectorXd Ff = engine.extractFreeLoadVectorPublic(F, freeDofs);
    
    std::cout << "\nFree stiffness matrix Kff (" << Kff.rows() << "x" << Kff.cols() << "):" << std::endl;
    std::cout << Kff << std::endl;
    
    std::cout << "\nFree load vector Ff:" << std::endl;
    std::cout << Ff.transpose() << std::endl;
    
    std::cout << "\nFree matrix determinant: " << Kff.determinant() << std::endl;
    
    return 0;
}
