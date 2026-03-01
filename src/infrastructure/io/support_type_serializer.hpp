/**
 * @file support_type_serializer.hpp
 * @brief Conversion helpers between core::SupportType and its string
 * representation used in JSON/XML files.
 *
 * Centralises logic that was previously duplicated in each reader and writer,
 * so a single place is updated when a new SupportType value is added.
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
