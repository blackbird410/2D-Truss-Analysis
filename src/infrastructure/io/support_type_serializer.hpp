/**
 * @file support_type_serializer.hpp
 * @brief Serializes and deserializes SupportType values to and from string.
 * @version 3.0.0
 * @date 2026-03-01
 * @author Neil Taison Rigaud
 */
#pragma once

#include "core/model/types.hpp"

#include <string>

/**
 * @brief Serialization helpers for @c core::SupportType within the I/O layer.
 */
namespace truss::infrastructure::io {

/**
 * @brief Parses a string token into the corresponding @c core::SupportType.
 *
 * Recognised tokens (case-sensitive): `"free"`, `"pinned"`, `"roller_x"`,
 * `"roller_y"`, and their legacy capitalised variants.
 *
 * @param  str  Token read from a JSON or XML file.
 * @return Corresponding @c core::SupportType value.
 * @throws ParseException if @p str is not a recognised token.
 */
[[nodiscard]] core::SupportType parseSupportType(const std::string& str);

/**
 * @brief Converts a @c core::SupportType value to its canonical string token.
 *
 * @param  type  Support type to convert.
 * @return Canonical lowercase token (`"free"`, `"pinned"`, `"roller_x"`, or `"roller_y"`).
 * @note   The returned token round-trips through parseSupportType().
 *         Unrecognised enum values fall back to `"free"`.
 */
[[nodiscard]] std::string supportTypeToString(core::SupportType type);

}  // namespace truss::infrastructure::io
