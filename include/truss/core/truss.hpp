/**
 * @file truss.hpp
 * @brief Main Truss structure class - represents complete 2D truss structure
 */

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace truss {

// Forward declarations
class Node;
class Member;
class Truss;

/**
 * @class Truss
 * @brief Represents a complete 2D truss structure
 *
 * Contains:
 * - Collection of nodes (joints)
 * - Collection of members (beams/columns)
 * - Support conditions
 * - Applied loads
 *
 * Used for structural analysis and validation
 */
class Truss {
public:
    virtual ~Truss() = default;

    /**
     * Get number of nodes
     * @return Node count
     */
    virtual std::size_t getNodeCount() const = 0;

    /**
     * Get number of members
     * @return Member count
     */
    virtual std::size_t getMemberCount() const = 0;

    /**
     * Check if truss structure is valid
     * @return True if structure is valid for analysis
     */
    virtual bool isValid() const = 0;
};

}  // namespace truss
