/**
 * @file TrussEditDTOs.hpp
 * @brief Application-layer Data Transfer Objects for GUI operations
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This file defines DTOs used by the Application layer to decouple GUI
 * Controllers from Domain types. These DTOs provide a stable interface
 * that isolates the GUI from Domain implementation changes.
 *
 * Architecture: Application Layer (Clean Architecture)
 * Purpose: Dependency Inversion - GUI depends on Application DTOs, not Domain
 */

#pragma once

#include <string>

namespace truss::application {

/**
 * @brief Material specification for member creation
 *
 * Application-layer value object representing material properties
 * without exposing Domain implementation details.
 */
struct MaterialSpec {
    double youngsModulusPa;  ///< Young's modulus in Pascals
    std::string name;        ///< Material name (e.g., "Steel", "Aluminum")

    /**
     * @brief Predefined steel material (E = 200 GPa)
     */
    static MaterialSpec Steel() { return {200e9, "Steel"}; }

    /**
     * @brief Predefined aluminum material (E = 69 GPa)
     */
    static MaterialSpec Aluminum() { return {69e9, "Aluminum"}; }

    /**
     * @brief Predefined concrete material (E = 30 GPa)
     */
    static MaterialSpec Concrete() { return {30e9, "Concrete"}; }
};

/**
 * @brief Cross-section specification for member creation
 *
 * Application-layer value object representing section properties
 * without exposing Domain implementation details.
 */
struct SectionSpec {
    double areaM2;        ///< Cross-sectional area in square meters
    std::string profile;  ///< Section profile description (e.g., "Square", "Circular")

    /**
     * @brief Create circular section from diameter
     * @param diameterM Diameter in meters
     */
    static SectionSpec Circular(double diameterM) {
        double area = 3.14159265359 * diameterM * diameterM / 4.0;
        return {area, "Circular"};
    }

    /**
     * @brief Create square section from side length
     * @param sideM Side length in meters
     */
    static SectionSpec Square(double sideM) {
        double area = sideM * sideM;
        return {area, "Square"};
    }
};

}  // namespace truss::application
