/**
 * @file truss_edit_dtos.hpp
 * @brief Application-layer Data Transfer Objects for GUI operations.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This file defines DTOs used by the Application layer to decouple GUI
 * Controllers from Domain types. These DTOs provide a stable interface
 * that isolates the GUI from Domain implementation changes.
 *
 * Architecture: Application Layer (Clean Architecture)
 * Purpose: Dependency Inversion - GUI depends on Application DTOs, not Domain
 */

#pragma once

#include <numbers>
#include <string>

namespace truss::application {

/**
 * @brief Material specification for member creation
 *
 * Application-layer value object representing material properties
 * without exposing Domain implementation details.
 */
struct MaterialSpec {
    double youngsModulusPa{0.0};  ///< Young's modulus in Pascals
    std::string name;             ///< Material name (e.g., "Steel", "Aluminum")

    /**
     * @brief Predefined steel material (E = 200 GPa)
     */
    [[maybe_unused]] static MaterialSpec Steel() {
        return {.youngsModulusPa = 200e9, .name = "Steel"};
    }

    /**
     * @brief Predefined aluminum material (E = 69 GPa)
     */
    [[maybe_unused]] static MaterialSpec Aluminum() {
        return {.youngsModulusPa = 69e9, .name = "Aluminum"};
    }

    /**
     * @brief Predefined concrete material (E = 30 GPa)
     */
    [[maybe_unused]] static MaterialSpec Concrete() {
        return {.youngsModulusPa = 30e9, .name = "Concrete"};
    }
};

/**
 * @brief Cross-section specification for member creation
 *
 * Application-layer value object representing section properties
 * without exposing Domain implementation details.
 */
struct SectionSpec {
    double areaM2{0.0};   ///< Cross-sectional area in square meters
    std::string profile;  ///< Section profile description (e.g., "Square", "Circular")

    /**
     * @brief Create circular section from diameter
     * @param diameterM Diameter in meters
     */
    [[maybe_unused]] static SectionSpec Circular(double diameterM) {
        double area = std::numbers::pi * diameterM * diameterM / 4.0;
        return {.areaM2 = area, .profile = "Circular"};
    }

    /**
     * @brief Create square section from side length
     * @param sideM Side length in meters
     */
    [[maybe_unused]] static SectionSpec Square(double sideM) {
        double area = sideM * sideM;
        return {.areaM2 = area, .profile = "Square"};
    }
};

/**
 * @brief Node update specification for position changes
 *
 * Application-layer DTO carrying the new position for an existing node.
 * The node's ID, support condition, and applied loads are NOT changed
 * by an updateNode operation — use setNodeSupport / applyNodeLoad for those.
 */
struct NodeUpdateSpec {
    double x{0.0};  ///< New X coordinate in metres
    double y{0.0};  ///< New Y coordinate in metres
};

/**
 * @brief Member update specification for material and section changes
 *
 * Application-layer DTO carrying new material and section properties for
 * an existing member.  The member's ID and node connectivity are preserved;
 * only material stiffness and cross-section area are replaced.
 */
struct MemberUpdateSpec {
    MaterialSpec material;  ///< New material specification (E in Pa, name)
    SectionSpec section;    ///< New section specification (area in m²)
};

}  // namespace truss::application
