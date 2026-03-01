/**
 * @file math_utils.hpp
 * @brief Pure mathematical utility functions with no project dependencies.
 *
 * This header is intentionally dependency-free: it only includes standard
 * library headers so it can be safely used by any layer without introducing
 * coupling.
 */
#pragma once

#include <cmath>
#include <numbers>

namespace truss::utils::math {

/// Default tolerance used for floating-point comparisons.
inline constexpr double DEFAULT_TOLERANCE = 1e-12;

/**
 * @brief Returns true if |value| < tolerance.
 */
inline bool isZero(double value, double tolerance = DEFAULT_TOLERANCE) noexcept {
    return std::abs(value) < tolerance;
}

/**
 * @brief Returns true if |a - b| < tolerance.
 */
inline bool isApproxEqual(double a, double b, double tolerance = DEFAULT_TOLERANCE) noexcept {
    return std::abs(a - b) < tolerance;
}

/**
 * @brief Converts degrees to radians.
 */
[[nodiscard]] inline double degreesToRadians(double degrees) noexcept {
    return degrees * std::numbers::pi / 180.0;
}

/**
 * @brief Converts radians to degrees.
 */
[[nodiscard]] inline double radiansToDegrees(double radians) noexcept {
    return radians * 180.0 / std::numbers::pi;
}

}  // namespace truss::utils::math
