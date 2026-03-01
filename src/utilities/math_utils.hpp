/**
 * @file math_utils.hpp
 * @brief Pure mathematical utility functions with no project dependencies.
 * @version 3.0.0
 * @date 2026-03-01
 * @author Neil Taison Rigaud
 */
#pragma once

#include <cmath>
#include <numbers>

/**
 * @brief Dependency-free floating-point comparison predicates and angle conversion utilities.
 *
 * This namespace carries no project-level dependencies and may be safely
 * included at any architectural layer.
 */
namespace truss::utils::math {

/// Default absolute tolerance used for floating-point comparisons.
inline constexpr double DEFAULT_TOLERANCE = 1e-12;

/**
 * @brief Returns `true` if the absolute value of @p value is below @p tolerance.
 * @param value     Value to test.
 * @param tolerance Maximum absolute magnitude considered zero. Defaults to
 *                  #DEFAULT_TOLERANCE.
 * @return `true` if `|value| < tolerance`, `false` otherwise.
 */
inline bool isZero(double value, double tolerance = DEFAULT_TOLERANCE) noexcept {
    return std::abs(value) < tolerance;
}

/**
 * @brief Returns `true` if @p a and @p b differ by less than @p tolerance.
 * @param a         First operand.
 * @param b         Second operand.
 * @param tolerance Maximum absolute difference considered equal. Defaults to
 *                  #DEFAULT_TOLERANCE.
 * @return `true` if `|a - b| < tolerance`, `false` otherwise.
 */
inline bool isApproxEqual(double a, double b, double tolerance = DEFAULT_TOLERANCE) noexcept {
    return std::abs(a - b) < tolerance;
}

/**
 * @brief Converts an angle from degrees to radians.
 * @param degrees Angle in degrees.
 * @return Equivalent angle in radians.
 */
[[nodiscard]] inline double degreesToRadians(double degrees) noexcept {
    return degrees * std::numbers::pi / 180.0;
}

/**
 * @brief Converts an angle from radians to degrees.
 * @param radians Angle in radians.
 * @return Equivalent angle in degrees.
 */
[[nodiscard]] inline double radiansToDegrees(double radians) noexcept {
    return radians * 180.0 / std::numbers::pi;
}

}  // namespace truss::utils::math
