/**
 * @file ITrussView.hpp
 * @brief Read-only view interface for Truss data
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
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
    NodeId id;
    Real x;
    Real y;
    SupportType support;
    Real fx{0.0};  ///< Applied force X-component
    Real fy{0.0};  ///< Applied force Y-component
    Real dx{0.0};  ///< Displacement X-component (from analysis results)
    Real dy{0.0};  ///< Displacement Y-component (from analysis results)
    Real rx{0.0};  ///< Reaction force X-component (from analysis results)
    Real ry{0.0};  ///< Reaction force Y-component (from analysis results)
};

/**
 * @brief Simple view struct for member data
 *
 * Provides read-only access to member properties without exposing
 * the full Member class interface to Infrastructure.
 */
struct MemberView {
    MemberId id;
    NodeId startNodeId;
    NodeId endNodeId;
    std::string label;

    // Material properties
    Real youngModulus;
    Real yieldStrength;
    Real density;

    // Section properties
    Real area;

    // Geometric properties (computed)
    Real length;
    Real angle;  ///< Angle in radians

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

    // Basic properties
    virtual const std::string& getName() const = 0;

    // Node access
    virtual std::vector<NodeView> getNodeViews() const = 0;
    virtual size_t getNodeCount() const = 0;

    // Member access
    virtual std::vector<MemberView> getMemberViews() const = 0;
    virtual size_t getMemberCount() const = 0;

    // System properties
    virtual size_t getTotalDofs() const = 0;
    virtual size_t getFreeDofs() const = 0;
    virtual size_t getConstrainedDofs() const = 0;
};

}  // namespace truss::core::interfaces
