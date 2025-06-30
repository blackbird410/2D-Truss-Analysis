#include "../../src/core/Truss.hpp"
#include "../../src/core/AnalysisEngine.hpp"
#include <iostream>
#include <iomanip>

using namespace truss::core;

int main() {
    std::cout << std::scientific << std::setprecision(3);
    
    // Create a simple truss
    Truss truss("Debug Test");
    
    std::cout << "Creating nodes..." << std::endl;
    auto node1 = truss.addNode(0.0, 0.0, SupportType::Pinned);
    auto node2 = truss.addNode(4.0, 0.0, SupportType::RollerY); 
    auto node3 = truss.addNode(2.0, 3.0, SupportType::Free);
    
    std::cout << "Node 1: (" << node1->getX() << ", " << node1->getY() << ") - Support: Pinned" << std::endl;
    std::cout << "Node 2: (" << node2->getX() << ", " << node2->getY() << ") - Support: RollerY" << std::endl;
    std::cout << "Node 3: (" << node3->getX() << ", " << node3->getY() << ") - Support: Free" << std::endl;
    
    // Add members and check their properties
    std::cout << "\nCreating members..." << std::endl;
    auto mem1 = truss.addMember(node1, node2);
    auto mem2 = truss.addMember(node1, node3);
    auto mem3 = truss.addMember(node2, node3);
    
    std::cout << "Member 1 length: " << mem1->getLength() << " m" << std::endl;
    std::cout << "Member 1 stiffness: " << mem1->getStiffness() << " N/m" << std::endl;
    std::cout << "Member 2 length: " << mem2->getLength() << " m" << std::endl;
    std::cout << "Member 2 stiffness: " << mem2->getStiffness() << " N/m" << std::endl;
    std::cout << "Member 3 length: " << mem3->getLength() << " m" << std::endl;
    std::cout << "Member 3 stiffness: " << mem3->getStiffness() << " N/m" << std::endl;
    
    // Apply load
    std::cout << "\nApplying load..." << std::endl;
    truss.applyForce(node3->getId(), Force2D(0.0, -10000.0));
    std::cout << "Applied 10 kN downward force to node 3" << std::endl;
    
    // Check DOF assignment
    truss.assignDofNumbers();
    std::cout << "\nDOF assignments:" << std::endl;
    std::cout << "Node 1 DOFs: X=" << node1->getDofX() << ", Y=" << node1->getDofY() << std::endl;
    std::cout << "Node 2 DOFs: X=" << node2->getDofX() << ", Y=" << node2->getDofY() << std::endl;
    std::cout << "Node 3 DOFs: X=" << node3->getDofX() << ", Y=" << node3->getDofY() << std::endl;
    std::cout << "Total DOFs: " << truss.getTotalDofs() << std::endl;
    std::cout << "Free DOFs: " << truss.getFreeDofs() << std::endl;
    
    // Create engine and get matrices
    AnalysisEngine engine;
    
    std::cout << "\nAssembling matrices..." << std::endl;
    MatrixXd K = engine.assembleStiffnessMatrix(truss);
    VectorXd F = engine.assembleLoadVector(truss);
    
    std::cout << "Stiffness matrix K (" << K.rows() << "x" << K.cols() << "):" << std::endl;
    std::cout << K << std::endl;
    
    std::cout << "\nLoad vector F:" << std::endl;
    std::cout << F.transpose() << std::endl;
    
    // Check matrix properties
    double det = K.determinant();
    std::cout << "\nMatrix determinant: " << det << std::endl;
    
    // Check diagonal values
    std::cout << "Diagonal values: ";
    for (int i = 0; i < K.rows(); ++i) {
        std::cout << K(i,i) << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
