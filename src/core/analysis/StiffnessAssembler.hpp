/**
 * @file StiffnessAssembler.hpp
 * @brief Assembles global stiffness matrices for truss structures
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This class is responsible for assembling the global stiffness matrix
 * from individual member stiffness matrices. It is part of the decomposition
 * of the monolithic AnalysisEngine class.
 */

#pragma once

#include "../model/Member.hpp"
#include "../model/Truss.hpp"
#include "../model/Types.hpp"

#include <Eigen/Dense>

#include <memory>

namespace truss::core::analysis {

/**
 * @brief Assembles global stiffness matrices for finite element analysis
 *
 * The StiffnessAssembler is responsible for constructing the global stiffness
 * matrix by iterating over all members in the truss and accumulating their
 * contributions based on their global DOF indices.
 *
 * Key responsibilities:
 * - Assemble global stiffness matrix from member contributions
 * - Handle coordinate transformations (delegated to Member class)
 * - Maintain numerical accuracy during assembly
 *
 * Design decisions:
 * - Uses Eigen::MatrixXd for dense matrix representation
 * - Assumes Member class provides getGlobalStiffnessMatrix()
 * - Returns matrix by value (move semantics for efficiency)
 */
class StiffnessAssembler {
public:
    /**
     * @brief Default constructor
     */
    StiffnessAssembler() = default;

    /**
     * @brief Assemble the global stiffness matrix for a truss
     * @param truss The truss structure to assemble
     * @return Global stiffness matrix (totalDofs × totalDofs)
     *
     * Algorithm:
     * 1. Initialize zero matrix of size totalDofs × totalDofs
     * 2. For each member:
     *    a. Get member's global stiffness matrix (4×4)
     *    b. Get member's global DOF indices
     *    c. Add member contribution to global matrix
     * 3. Return assembled matrix
     *
     * Complexity: O(n * d^2) where n = members, d = DOFs per member (4)
     * Memory: O(totalDofs^2)
     */
    MatrixXd assemble(const Truss& truss) const;

    /**
     * @brief Assemble stiffness matrix and return as 2D vector (for display)
     * @param truss The truss structure
     * @return Stiffness matrix as 2D vector of Reals
     *
     * This method is useful for exporting or displaying the matrix,
     * as it converts from Eigen format to standard containers.
     */
    std::vector<std::vector<Real>> assembleAsVector(const Truss& truss) const;

private:
    /**
     * @brief Add a single member's contribution to the global matrix
     * @param member The member to add
     * @param globalK The global stiffness matrix (modified in place)
     *
     * This method extracts the member's global stiffness matrix and
     * global DOF indices, then adds the member's contribution to the
     * appropriate locations in the global matrix.
     */
    void addMemberContribution(const Member& member, MatrixXd& globalK) const;
};

// Type aliases
using StiffnessAssemblerPtr = std::shared_ptr<StiffnessAssembler>;
using StiffnessAssemblerConstPtr = std::shared_ptr<const StiffnessAssembler>;

}  // namespace truss::core::analysis
