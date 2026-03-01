/**
 * @file test_math_utils.cpp
 * @brief Unit tests for truss::utils::math utility functions.
 */

#include "utilities/math_utils.hpp"

#include <gtest/gtest.h>
#include <numbers>

using namespace truss::utils::math;

// ─── isZero ──────────────────────────────────────────────────────────────────

TEST(MathUtilsTest, IsZero_ExactZero_ReturnsTrue) {
    EXPECT_TRUE(isZero(0.0));
}

TEST(MathUtilsTest, IsZero_SmallPositive_ReturnsFalse) {
    EXPECT_FALSE(isZero(1e-11));
}

TEST(MathUtilsTest, IsZero_ValueWithinDefaultTolerance_ReturnsTrue) {
    EXPECT_TRUE(isZero(1e-13));
}

TEST(MathUtilsTest, IsZero_NegativeWithinTolerance_ReturnsTrue) {
    EXPECT_TRUE(isZero(-1e-13));
}

TEST(MathUtilsTest, IsZero_CustomTolerance) {
    EXPECT_TRUE(isZero(0.001, 0.01));
    EXPECT_FALSE(isZero(0.001, 0.0001));
}

// ─── isApproxEqual ───────────────────────────────────────────────────────────

TEST(MathUtilsTest, IsApproxEqual_SameValue_ReturnsTrue) {
    EXPECT_TRUE(isApproxEqual(3.14159, 3.14159));
}

TEST(MathUtilsTest, IsApproxEqual_SlightlyDifferent_ReturnsFalse) {
    EXPECT_FALSE(isApproxEqual(1.0, 1.0 + 1e-11));
}

TEST(MathUtilsTest, IsApproxEqual_WithinCustomTolerance) {
    EXPECT_TRUE(isApproxEqual(1.0, 1.0001, 0.001));
    EXPECT_FALSE(isApproxEqual(1.0, 1.01, 0.001));
}

// ─── degreesToRadians ────────────────────────────────────────────────────────

TEST(MathUtilsTest, DegreesToRadians_Zero) {
    EXPECT_DOUBLE_EQ(degreesToRadians(0.0), 0.0);
}

TEST(MathUtilsTest, DegreesToRadians_90Degrees) {
    EXPECT_NEAR(degreesToRadians(90.0), std::numbers::pi / 2.0, 1e-12);
}

TEST(MathUtilsTest, DegreesToRadians_180Degrees) {
    EXPECT_NEAR(degreesToRadians(180.0), std::numbers::pi, 1e-12);
}

TEST(MathUtilsTest, DegreesToRadians_360Degrees) {
    EXPECT_NEAR(degreesToRadians(360.0), 2.0 * std::numbers::pi, 1e-12);
}

// ─── radiansToDegrees ────────────────────────────────────────────────────────

TEST(MathUtilsTest, RadiansToDegrees_Zero) {
    EXPECT_DOUBLE_EQ(radiansToDegrees(0.0), 0.0);
}

TEST(MathUtilsTest, RadiansToDegrees_PI) {
    EXPECT_NEAR(radiansToDegrees(std::numbers::pi), 180.0, 1e-10);
}

TEST(MathUtilsTest, RadiansToDegrees_HalfPI) {
    EXPECT_NEAR(radiansToDegrees(std::numbers::pi / 2.0), 90.0, 1e-10);
}

// ─── Round-trip ──────────────────────────────────────────────────────────────

TEST(MathUtilsTest, DegreesRadiansRoundTrip_45) {
    EXPECT_NEAR(radiansToDegrees(degreesToRadians(45.0)), 45.0, 1e-10);
}

TEST(MathUtilsTest, DegreesRadiansRoundTrip_Negative) {
    EXPECT_NEAR(radiansToDegrees(degreesToRadians(-30.0)), -30.0, 1e-10);
}
