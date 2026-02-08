/**
 * @file BoundaryConditionHandler.hpp
 * @brief Handles boundary condition application for structural analysis
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "../model/Types.hpp"
#include "../model/Truss.hpp"
#include <Eigen/Dense>
#include <vector>

namespace truss::core::analysis {

using Eigen::MatrixXd;
using Eigen::VectorXd;

/**
 * @brief Manages boundary condition application for finite element analysis
 * 
 * This class encapsulates the logic for:
 * - Identifying free and constrained degrees of freedom based on support types
 * - Extracting the reduced system (free DOFs only) from global matrices
 * - Expanding solution vectors to include constrained DOFs (zero displacements)
 * 
 * The boundary condition handler supports all standard 2D truss support types:
 * - Free: Both DOFs unconstrained
 * - PinnedX: X DOF constrained, Y DOF free
 * - PinnedY: Y DOF constrained, X DOF free  
 * - Pinned: Both DOFs constrained
 * - RollerX: Y DOF constrained, X DOF free
 * - RollerY: X DOF constrained, Y DOF free
 * 
 * @note All methods are const and stateless - this is a pure utility class
 * 
 * @see AnalysisEngine Original implementation location
 * @see SupportType Enumeration of support constraint types
 */
class BoundaryConditionHandler {
public:
    /**
     * @brief Default constructor
     */
    BoundaryConditionHandler() = default;

    /**
     * @brief Get indices of free (unconstrained) degrees of freedom
     * 
     * Examines each node's support type to determine which DOFs are free to
     * displace. The returned indices correspond to the global DOF numbering
     * assigned by Truss::assignDofNumbers().
     * 
     * @param truss The truss structure with assigned DOF numbers
     * @return Sorted vector of free DOF indices
     * 
     * @note Requires truss.assignDofNumbers() to be called first
     * @note Returned indices are sorted in ascending order
     * 
     * Example:
     * @code
     * BoundaryConditionHandler handler;
     * auto freeDofs = handler.getFreeDofs(truss);
     * // freeDofs might be [0, 1, 2, 5, 6, 7] for a 4-node truss with 2 pinned supports
     * @endcode
     */
    std::vector<Index> getFreeDofs(const Truss& truss) const;

    /**
     * @brief Get indices of constrained (fixed) degrees of freedom
     * 
     * Examines each node's support type to determine which DOFs are constrained
     * (zero displacement). The returned indices correspond to the global DOF
     * numbering assigned by Truss::assignDofNumbers().
     * 
     * @param truss The truss structure with assigned DOF numbers
     * @return Sorted vector of constrained DOF indices
     * 
     * @note Requires truss.assignDofNumbers() to be called first
     * @note Returned indices are sorted in ascending order
     * 
     * Example:
     * @code
     * BoundaryConditionHandler handler;
     * auto constrainedDofs = handler.getConstrainedDofs(truss);
     * // constrainedDofs might be [3, 4, 8, 9] for a 5-node truss with 2 pinned supports
     * @endcode
     */
    std::vector<Index> getConstrainedDofs(const Truss& truss) const;

    /**
     * @brief Apply boundary conditions to global stiffness matrix
     * 
     * Extracts the reduced stiffness matrix containing only the rows and columns
     * corresponding to free DOFs. This produces the system Kff that can be solved.
     * 
     * Mathematical Operation:
     * @code
     * Given: K (n×n global stiffness), freeDofs (m indices where m < n)
     * Returns: Kff (m×m reduced stiffness)
     * where Kff(i,j) = K(freeDofs[i], freeDofs[j])
     * @endcode
     * 
     * @param K Global stiffness matrix (totalDofs × totalDofs)
     * @param freeDofs Indices of free DOFs
     * @return Reduced stiffness matrix (freeDofs.size() × freeDofs.size())
     * 
     * @note Matrix K must be square with dimensions matching total DOFs
     * @note freeDofs indices must be within bounds [0, K.rows())
     * 
     * Example:
     * @code
     * MatrixXd K = assembler.assemble(truss); // 10×10 matrix
     * auto freeDofs = handler.getFreeDofs(truss); // [0,1,2,3,4,5]
     * MatrixXd Kff = handler.applyToStiffness(K, freeDofs); // 6×6 matrix
     * @endcode
     */
    MatrixXd applyToStiffness(const MatrixXd& K, const std::vector<Index>& freeDofs) const;

    /**
     * @brief Apply boundary conditions to global load vector
     * 
     * Extracts the reduced load vector containing only the entries corresponding
     * to free DOFs. This produces the system Ff that can be solved.
     * 
     * Mathematical Operation:
     * @code
     * Given: F (n×1 global load), freeDofs (m indices where m < n)
     * Returns: Ff (m×1 reduced load)
     * where Ff(i) = F(freeDofs[i])
     * @endcode
     * 
     * @param F Global load vector (totalDofs × 1)
     * @param freeDofs Indices of free DOFs
     * @return Reduced load vector (freeDofs.size() × 1)
     * 
     * @note Vector F must have dimensions matching total DOFs
     * @note freeDofs indices must be within bounds [0, F.rows())
     * 
     * Example:
     * @code
     * VectorXd F = assembler.assembleLoads(truss); // 10×1 vector
     * auto freeDofs = handler.getFreeDofs(truss); // [0,1,2,3,4,5]
     * VectorXd Ff = handler.applyToLoad(F, freeDofs); // 6×1 vector
     * @endcode
     */
    VectorXd applyToLoad(const VectorXd& F, const std::vector<Index>& freeDofs) const;

    /**
     * @brief Expand free DOF displacements to full displacement vector
     * 
     * Creates the full displacement vector by placing the solved free DOF
     * displacements at their correct global positions and setting constrained
     * DOFs to zero.
     * 
     * Mathematical Operation:
     * @code
     * Given: Uf (m×1 free displacements), freeDofs (m indices), totalDofs (n)
     * Returns: U (n×1 full displacements)
     * where:
     *   U(freeDofs[i]) = Uf(i)  for all i
     *   U(j) = 0                for all constrained DOFs j
     * @endcode
     * 
     * @param freeSolution Displacements for free DOFs only (solution from solver)
     * @param freeDofs Indices of free DOFs
     * @param totalDofs Total number of DOFs in the system
     * @return Full displacement vector (totalDofs × 1) with zeros for constrained DOFs
     * 
     * @note freeSolution.size() must equal freeDofs.size()
     * @note All indices in freeDofs must be < totalDofs
     * 
     * Example:
     * @code
     * VectorXd Uf(6); // Solved displacements for 6 free DOFs
     * auto freeDofs = handler.getFreeDofs(truss); // [0,1,2,3,4,5]
     * VectorXd U = handler.expandDisplacements(Uf, freeDofs, 10); // 10×1 full vector
     * // U will have Uf values at indices 0-5, zeros at indices 6-9
     * @endcode
     */
    VectorXd expandDisplacements(const VectorXd& freeSolution,
                                  const std::vector<Index>& freeDofs,
                                  size_t totalDofs) const;
};

} // namespace truss::core::analysis
