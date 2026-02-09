/**
 * @file Load.hpp
 * @brief Load class representing external forces applied to the truss
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "Types.hpp"
#include <string>
#include <memory>

namespace truss::core {

/**
 * @brief Type of load application
 */
enum class LoadType {
    NodalForce,         ///< Concentrated force at a node
    DistributedLoad,    ///< Distributed load along a member (future)
    SelfWeight,         ///< Self-weight of members (future)
    Temperature         ///< Temperature-induced load (future)
};

/**
 * @brief Represents an external load applied to the truss structure
 * 
 * A load encapsulates information about forces applied to nodes,
 * including magnitude, direction, and type. This provides better
 * encapsulation than directly storing forces in nodes.
 */
class Load {
public:
    /**
     * @brief Construct a nodal force load
     * @param id Unique identifier for the load
     * @param nodeId Node where force is applied
     * @param force Force vector
     * @param label Optional descriptive label
     */
    Load(LoadId id, NodeId nodeId, const Force2D& force, const std::string& label = "");
    
    /**
     * @brief Construct a nodal force load with separate components
     * @param id Unique identifier for the load
     * @param nodeId Node where force is applied
     * @param fx Force in X direction
     * @param fy Force in Y direction
     * @param label Optional descriptive label
     */
    Load(LoadId id, NodeId nodeId, Real fx, Real fy, const std::string& label = "");
    
    // Copy and move semantics
    Load(const Load& other) = default;
    Load(Load&& other) noexcept = default;
    Load& operator=(const Load& other) = default;
    Load& operator=(Load&& other) noexcept = default;
    ~Load() = default;
    
    // Getters
    LoadId getId() const noexcept { return m_id; }
    NodeId getNodeId() const noexcept { return m_nodeId; }
    LoadType getType() const noexcept { return m_type; }
    const Force2D& getForce() const noexcept { return m_force; }
    Real getFx() const noexcept { return m_force.fx; }
    Real getFy() const noexcept { return m_force.fy; }
    const std::string& getLabel() const noexcept { return m_label; }
    Real getMagnitude() const noexcept { return m_force.magnitude(); }
    
    // Setters
    void setId(LoadId id) { m_id = id; }
    void setNodeId(NodeId nodeId) { m_nodeId = nodeId; }
    void setForce(const Force2D& force) { m_force = force; }
    void setForce(Real fx, Real fy) { m_force = Force2D(fx, fy); }
    void setLabel(const std::string& label) { m_label = label; }
    
    // Queries
    bool isZero(Real tolerance = Constants::FORCE_TOLERANCE) const;
    bool isHorizontal(Real tolerance = Constants::FORCE_TOLERANCE) const;
    bool isVertical(Real tolerance = Constants::FORCE_TOLERANCE) const;
    bool isAppliedAt(NodeId nodeId) const { return m_nodeId == nodeId; }
    
    // Operators
    bool operator==(const Load& other) const;
    bool operator!=(const Load& other) const;

private:
    LoadId m_id;               ///< Unique identifier
    NodeId m_nodeId;           ///< Node where load is applied
    LoadType m_type;           ///< Type of load
    Force2D m_force;           ///< Force vector
    std::string m_label;       ///< Descriptive label
};

// Type alias for load pointer
using LoadPtr = std::shared_ptr<Load>;
using LoadVector = std::vector<LoadPtr>;

} // namespace truss::core
