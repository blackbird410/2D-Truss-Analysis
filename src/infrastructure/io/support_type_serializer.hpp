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

namespace truss::infrastructure::io {

/**
 * @brief Parses a string token (e.g. "pinned", "roller_x") into a
 * core::SupportType.
 *
 * @throws ParseException if the token is not recognised.
 */
[[nodiscard]] core::SupportType parseSupportType(const std::string& str);

/**
 * @brief Converts a core::SupportType value to its canonical string token.
 *
 * The returned token round-trips through parseSupportType.  Unknown values
 * fall back to "free".
 */
[[nodiscard]] std::string supportTypeToString(core::SupportType type);

}  // namespace truss::infrastructure::io
