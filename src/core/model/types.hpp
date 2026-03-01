/**
 * @file types.hpp
 * @brief Core data types and mathematical utilities for truss analysis.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include <Eigen/Dense>

#include <cstdint>
#include <memory>
#include <vector>

#include "utilities/math_utils.hpp"

namespace truss::core {

// Type aliases for better readability
using Real = double;
using Index = std::size_t;
using NodeId = std::uint32_t;
using MemberId = std::uint32_t;
using LoadId = std::uint32_t;

// Eigen type aliases
using Vector2d = Eigen::Vector2d;
using Vector3d = Eigen::Vector3d;
using Matrix2d = Eigen::Matrix2d;
using MatrixXd = Eigen::MatrixXd;
using VectorXd = Eigen::VectorXd;

/**
 * @brief 2D Point structure
 */
struct Point2D {
    Real x{0.0};
    Real y{0.0};

    Point2D() = default;
    Point2D(Real x_val, Real y_val) : x(x_val), y(y_val) {}

    Point2D operator+(const Point2D& other) const { return Point2D(x + other.x, y + other.y); }

    Point2D operator-(const Point2D& other) const { return Point2D(x - other.x, y - other.y); }

    Point2D operator*(Real scalar) const { return Point2D(x * scalar, y * scalar); }

    Real distance(const Point2D& other) const {
        Real dx = x - other.x;
        Real dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    Vector2d toEigen() const { return Vector2d(x, y); }
};

/**
 * @brief 2D Force vector
 */
struct Force2D {
    Real fx{0.0};  ///< Force component in X direction
    Real fy{0.0};  ///< Force component in Y direction

    Force2D() = default;
    Force2D(Real fx_val, Real fy_val) : fx(fx_val), fy(fy_val) {}

    Force2D operator+(const Force2D& other) const { return Force2D(fx + other.fx, fy + other.fy); }

    Force2D operator-(const Force2D& other) const { return Force2D(fx - other.fx, fy - other.fy); }

    Force2D operator*(Real scalar) const { return Force2D(fx * scalar, fy * scalar); }

    Real magnitude() const { return std::sqrt(fx * fx + fy * fy); }

    Vector2d toEigen() const { return Vector2d(fx, fy); }
};

/**
 * @brief Support constraint types
 *
 * IMPORTANT: In 2D structural mechanics, a pinned support restrains BOTH translational DOFs.
 * Directional constraints are represented by roller supports.
 * Do NOT introduce directional pinning (e.g., PinnedX, PinnedY) - this is mechanically invalid.
 */
enum class SupportType {
    Free,     ///< No constraint (2 DOFs free)
    Pinned,   ///< Pin support: Fixed in both X and Y directions (0 DOFs free, 2 reactions)
    RollerX,  ///< Roller allowing movement in X direction (1 DOF free: X, 1 reaction: Y)
    RollerY   ///< Roller allowing movement in Y direction (1 DOF free: Y, 1 reaction: X)
};

/**
 * @brief Material properties for structural members
 */
struct MaterialProperties {
    Real youngModulus{200e9};      ///< Young's modulus (Pa)
    Real density{7850.0};          ///< Material density (kg/m³)
    Real yieldStrength{250e6};     ///< Yield strength (Pa)
    Real ultimateStrength{400e6};  ///< Ultimate tensile strength (Pa)
    std::string name{"Steel"};     ///< Material name

    MaterialProperties() = default;
    MaterialProperties(Real E, Real rho, Real fy, Real fu, const std::string& materialName)
        : youngModulus(E), density(rho), yieldStrength(fy), ultimateStrength(fu),
          name(materialName) {}
};

/**
 * @brief Cross-sectional properties for structural members
 */
struct SectionProperties {
    Real area{1e-4};                     ///< Cross-sectional area (m²)
    Real momentOfInertia{1e-8};          ///< Second moment of area (m⁴)
    Real shearArea{1e-4};                ///< Effective shear area (m²)
    std::string designation{"Default"};  ///< Section designation

    SectionProperties() = default;
    SectionProperties(Real A, Real I, Real As, const std::string& desig)
        : area(A), momentOfInertia(I), shearArea(As), designation(desig) {}
};

/**
 * @brief Analysis results for a single member
 */
struct MemberResults {
    Real axialForce{0.0};        ///< Axial force (positive = tension)
    Real axialStress{0.0};       ///< Axial stress
    Real utilizationRatio{0.0};  ///< Stress/yield stress ratio
    bool inTension{false};       ///< True if member is in tension
    bool yielded{false};         ///< True if member has yielded

    MemberResults() = default;
};

/**
 * @brief Analysis results for a single node
 */
struct NodeResults {
    Point2D displacement{0.0, 0.0};  ///< Nodal displacement
    Force2D reaction{0.0, 0.0};      ///< Support reaction forces

    NodeResults() = default;
};

/**
 * @brief Constants and tolerances for numerical analysis
 */
namespace Constants {
constexpr Real ZERO_TOLERANCE = 1e-12;
constexpr Real GEOMETRY_TOLERANCE = 1e-9;
constexpr Real FORCE_TOLERANCE = 1e-6;
constexpr Real DISPLACEMENT_TOLERANCE = 1e-9;
constexpr Real DEFAULT_YOUNG_MODULUS = 200e9;  // Steel (Pa)
constexpr Real DEFAULT_AREA = 1e-4;            // 1 cm² (m²)
}  // namespace Constants

}  // namespace truss::core
