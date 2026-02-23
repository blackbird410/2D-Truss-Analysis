/**
 * @file node.hpp
 * @brief Node class definition - represents a point in the truss structure
 */

#pragma once

#include <cstdint>
#include <vector>

namespace truss {

// Forward declaration
class Node;
using NodeId = std::uint32_t;

/**
 * @class Node
 * @brief Represents a joint node in a 2D truss structure
 * 
 * A node is defined by:
 * - Unique identifier
 * - X, Y coordinates in 2D space
 * - Optional constraints (fixed, pinned, roller support)
 * - Connected members
 */
class Node {
public:
    // Public interface - minimal surface for external users
    // Implementation details are in src/core/model/node.hpp/cpp
    
    virtual ~Node() = default;
    
    /**
     * Get node identifier
     * @return Unique node ID
     */
    virtual NodeId getId() const = 0;
    
    /**
     * Get X coordinate
     * @return X position in meters
     */
    virtual double getX() const = 0;
    
    /**
     * Get Y coordinate
     * @return Y position in meters
     */
    virtual double getY() const = 0;
};

} // namespace truss
