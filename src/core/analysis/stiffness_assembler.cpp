/**
 * @file stiffness_assembler.cpp
 * @brief Constructs global stiffness matrix from member contributions.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "stiffness_assembler.hpp"

namespace truss::core::analysis {

MatrixXd StiffnessAssembler::assemble(const Truss& truss) {
    // Initialize zero matrix
    size_t totalDofs = truss.getTotalDofs();
    MatrixXd K = MatrixXd::Zero(totalDofs, totalDofs);

    // Assemble contributions from all members
    const auto& members = truss.getMembers();
    for (const auto& member : members) {
        addMemberContribution(*member, K);
    }

    return K;
}

[[maybe_unused]] std::vector<std::vector<Real>>
StiffnessAssembler::assembleAsVector(const Truss& truss) const {
    MatrixXd K = assemble(truss);

    size_t rows = K.rows();
    size_t cols = K.cols();

    std::vector<std::vector<Real>> result(rows, std::vector<Real>(cols));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[i][j] = K(i, j);
        }
    }

    return result;
}

// cppcheck-suppress constParameterReference
void StiffnessAssembler::addMemberContribution(const Member& member, MatrixXd& globalK) {
    // Get member's global stiffness matrix and DOF indices
    auto dofIndices = member.getGlobalDofIndices();
    MatrixXd memberK = member.getGlobalStiffnessMatrix();

    // Add member stiffness to global matrix
    // Member has 4 DOFs: [u1, v1, u2, v2] corresponding to start and end nodes
    for (size_t i = 0; i < dofIndices.size(); ++i) {
        for (size_t j = 0; j < dofIndices.size(); ++j) {
            globalK(dofIndices[i], dofIndices[j]) += memberK(i, j);
        }
    }
}

}  // namespace truss::core::analysis
