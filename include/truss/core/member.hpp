/**
 * @file member.hpp
 * @brief Member class definition - represents a structural member connecting nodes
 */

#pragma once

#include <cstdint>

namespace truss {

// Forward declarations
class Member;
class Node;
using MemberId = std::uint32_t;

/**
 * @class Member
 * @brief Represents a truss member (beam/column) connecting two nodes
 *
 * A member is defined by:
 * - Unique identifier
 * - Start and end nodes
 * - Cross-sectional area
 * - Material properties (Young's modulus)
 * - Computed properties: length, axial force, strain
 */
class Member {
public:
    virtual ~Member() = default;

    /**
     * Get member identifier
     * @return Unique member ID
     */
    virtual MemberId getId() const = 0;

    /**
     * Get member length
     * @return Length in meters
     */
    virtual double getLength() const = 0;

    /**
     * Get cross-sectional area
     * @return Area in m²
     */
    virtual double getArea() const = 0;

    /**
     * Get Young's modulus (stiffness)
     * @return Modulus in Pa
     */
    virtual double getYoungsModulus() const = 0;
};

}  // namespace truss
