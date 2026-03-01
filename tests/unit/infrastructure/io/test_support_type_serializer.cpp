/**
 * @file test_support_type_serializer.cpp
 * @brief Unit tests for parseSupportType() and supportTypeToString().
 * @version 3.0.0
 * @date 2026-03-01
 * @author Neil Taison Rigaud
 */

#include "infrastructure/io/io_types.hpp"
#include "infrastructure/io/support_type_serializer.hpp"

#include <gtest/gtest.h>

using namespace truss::infrastructure::io;
using namespace truss::core;

// ─── parseSupportType ────────────────────────────────────────────────────────

TEST(SupportTypeSerializerTest, ParseFree_LowercaseAndCapitalized) {
    EXPECT_EQ(parseSupportType("free"), SupportType::Free);
    EXPECT_EQ(parseSupportType("Free"), SupportType::Free);
}

TEST(SupportTypeSerializerTest, ParsePinned_LowercaseAndCapitalized) {
    EXPECT_EQ(parseSupportType("pinned"), SupportType::Pinned);
    EXPECT_EQ(parseSupportType("Pinned"), SupportType::Pinned);
}

TEST(SupportTypeSerializerTest, ParseFixed_MapsToPin) {
    // "fixed" is treated as Pinned (all-DOF constraint)
    EXPECT_EQ(parseSupportType("fixed"), SupportType::Pinned);
    EXPECT_EQ(parseSupportType("Fixed"), SupportType::Pinned);
}

TEST(SupportTypeSerializerTest, ParseRollerX) {
    EXPECT_EQ(parseSupportType("roller_x"), SupportType::RollerX);
    EXPECT_EQ(parseSupportType("RollerX"), SupportType::RollerX);
}

TEST(SupportTypeSerializerTest, ParseRollerY) {
    EXPECT_EQ(parseSupportType("roller_y"), SupportType::RollerY);
    EXPECT_EQ(parseSupportType("RollerY"), SupportType::RollerY);
}

TEST(SupportTypeSerializerTest, ParseGenericRoller_MapsToRollerX) {
    EXPECT_EQ(parseSupportType("roller"), SupportType::RollerX);
    EXPECT_EQ(parseSupportType("Roller"), SupportType::RollerX);
}

TEST(SupportTypeSerializerTest, ParsePinnedX_MapsToRollerY) {
    // pinned_x: X is constrained → Y is free → RollerY
    EXPECT_EQ(parseSupportType("pinned_x"), SupportType::RollerY);
    EXPECT_EQ(parseSupportType("PinnedX"), SupportType::RollerY);
}

TEST(SupportTypeSerializerTest, ParsePinnedY_MapsToRollerX) {
    // pinned_y: Y is constrained → X is free → RollerX
    EXPECT_EQ(parseSupportType("pinned_y"), SupportType::RollerX);
    EXPECT_EQ(parseSupportType("PinnedY"), SupportType::RollerX);
}

TEST(SupportTypeSerializerTest, ParseUnknown_ThrowsParseException) {
    EXPECT_THROW((void)parseSupportType("unknown"), ParseException);
    EXPECT_THROW((void)parseSupportType(""), ParseException);
    EXPECT_THROW((void)parseSupportType("PINNED"), ParseException);
}

// ─── supportTypeToString ─────────────────────────────────────────────────────

TEST(SupportTypeSerializerTest, ToStringFree) {
    EXPECT_EQ(supportTypeToString(SupportType::Free), "free");
}

TEST(SupportTypeSerializerTest, ToStringPinned) {
    EXPECT_EQ(supportTypeToString(SupportType::Pinned), "pinned");
}

TEST(SupportTypeSerializerTest, ToStringRollerX) {
    EXPECT_EQ(supportTypeToString(SupportType::RollerX), "roller_x");
}

TEST(SupportTypeSerializerTest, ToStringRollerY) {
    EXPECT_EQ(supportTypeToString(SupportType::RollerY), "roller_y");
}

// ─── Round-trip ──────────────────────────────────────────────────────────────

TEST(SupportTypeSerializerTest, RoundTrip_Free) {
    EXPECT_EQ(parseSupportType(supportTypeToString(SupportType::Free)), SupportType::Free);
}

TEST(SupportTypeSerializerTest, RoundTrip_Pinned) {
    EXPECT_EQ(parseSupportType(supportTypeToString(SupportType::Pinned)), SupportType::Pinned);
}

TEST(SupportTypeSerializerTest, RoundTrip_RollerX) {
    EXPECT_EQ(parseSupportType(supportTypeToString(SupportType::RollerX)), SupportType::RollerX);
}

TEST(SupportTypeSerializerTest, RoundTrip_RollerY) {
    EXPECT_EQ(parseSupportType(supportTypeToString(SupportType::RollerY)), SupportType::RollerY);
}
