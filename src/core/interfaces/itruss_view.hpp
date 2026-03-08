/**
 * @file itruss_view.hpp
 * @brief Read-only view interface for Truss data.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This interface provides Infrastructure layer with read-only access to
 * Truss data, enforcing Dependency Inversion Principle (DIP).
 * Infrastructure depends on this abstraction, not concrete Domain types.
 */

#pragma once

#include "../model/types.hpp"

#include <memory>
#include <string>
#include <vector>

namespace truss::core::interfaces {

/**
 * @brief Simple view struct for node data
 *
 * Provides read-only access to node properties without exposing
 * the full Node class interface to Infrastructure.
 */
struct NodeView {
    NodeId id{0};                   ///< Unique node identifier
    Real x{0.0};                    ///< World-space X coordinate (metres, X+ rightward)
    Real y{0.0};                    ///< World-space Y coordinate (metres, Y+ upward)
    SupportType support{SupportType::Free};  ///< Boundary-condition type (Free / Pinned / RollerX / RollerY)
    Real fx{0.0};         ///< Applied force X-component (N)
    Real fy{0.0};         ///< Applied force Y-component (N)
    Real dx{0.0};         ///< Displacement X-component (metres, from analysis results)
    Real dy{0.0};         ///< Displacement Y-component (metres, from analysis results)
    Real rx{0.0};  ///< Reaction force X-component (N, from analysis results; positive = rightward)
    Real ry{0.0};  ///< Reaction force Y-component (N, from analysis results; positive = upward)
};

/**
 * @brief Simple view struct for member data
 *
 * Provides read-only access to member properties without exposing
 * the full Member class interface to Infrastructure.
 */
struct MemberView {
    MemberId id{0};         ///< Unique member identifier
    NodeId startNodeId{0};  ///< ID of the member's start node
    NodeId endNodeId{0};    ///< ID of the member's end node
    std::string label;     ///< Optional display label

    // Material properties
    Real youngModulus{0.0};   ///< Young's modulus E (Pa)
    Real yieldStrength{0.0};  ///< Yield strength f_y (Pa)
    Real density{0.0};        ///< Mass density ρ (kg/m³)

    // Section properties
    Real area{0.0};  ///< Cross-sectional area A (m²)

    // Geometric properties (computed)
    Real length{0.0};  ///< Member length L (metres)
    Real angle{0.0};   ///< Angle with respect to global X-axis (radians)

    // Analysis results
    Real axialForce{0.0};
    Real axialStress{0.0};
    Real utilizationRatio{0.0};
    bool inTension{false};
    bool yielded{false};
};

/**
 * @brief Read-only view interface for Truss
 *
 * This interface allows Infrastructure layer (exporters, writers)
 * to access Truss data without depending on the concrete Truss class.
 *
 * Benefits:
 * - Infrastructure depends on abstraction, not concrete type (DIP)
 * - Domain controls what data is exposed to Infrastructure
 * - Changes to Truss internals don't force Infrastructure recompilation
 */
class ITrussView {
public:
    virtual ~ITrussView() = default;

    // ================================================================
    // Basic properties
    // ================================================================

    /**
     * @brief Display name of the truss model.
     * @return Reference to the truss name string (valid for the lifetime of the object).
     */
    virtual const std::string& getName() const = 0;

    // ================================================================
    // Node access
    // ================================================================

    /**
     * @brief Snapshot of all node data for read-only consumption.
     * @return Vector of NodeView structs, one per node, in insertion order.
     */
    virtual std::vector<NodeView> getNodeViews() const = 0;

    /**
     * @brief Number of nodes currently in the truss.
     * @return Node count.
     */
    virtual size_t getNodeCount() const = 0;

    // ================================================================
    // Member access
    // ================================================================

    /**
     * @brief Snapshot of all member data for read-only consumption.
     * @return Vector of MemberView structs, one per member, in insertion order.
     */
    virtual std::vector<MemberView> getMemberViews() const = 0;

    /**
     * @brief Number of members currently in the truss.
     * @return Member count.
     */
    virtual size_t getMemberCount() const = 0;

    // ================================================================
    // System DOF properties
    // ================================================================

    /**
     * @brief Total number of degrees of freedom (2 per node).
     * @return 2 × node count.
     */
    virtual size_t getTotalDofs() const = 0;

    /**
     * @brief Number of unconstrained (free) degrees of freedom.
     * @return Total DOFs minus constrained DOFs.
     */
    virtual size_t getFreeDofs() const = 0;

    /**
     * @brief Number of constrained degrees of freedom (DOFs eliminated by supports).
     * @return Count of constrained DOFs.
     */
    virtual size_t getConstrainedDofs() const = 0;
};

}  // namespace truss::core::interfaces
