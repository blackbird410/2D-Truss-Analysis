/**
 * @file string_utils.hpp
 * @brief String formatting and escaping utilities with no project dependencies.
 *
 * All functions in this header depend only on the C++ standard library so any
 * layer can safely include it.
 */
#pragma once

#include <string>

namespace truss::utils::string {

// ─── Formatting ──────────────────────────────────────────────────────────────

/**
 * @brief Formats a floating-point number as a string.
 * @param value      The value to format.
 * @param precision  Number of digits after the decimal point.
 * @param scientific Use scientific notation when true, fixed otherwise.
 */
[[nodiscard]] std::string formatReal(double value, int precision, bool scientific = false);

/**
 * @brief Returns the current wall-clock time as "YYYY-MM-DD HH:MM:SS".
 */
[[nodiscard]] std::string formatTimestamp();

// ─── Case conversion ─────────────────────────────────────────────────────────

/**
 * @brief Returns a copy of @p s with all ASCII letters converted to lowercase.
 */
[[nodiscard]] std::string toLower(std::string s);

/**
 * @brief Returns a copy of @p s with all ASCII letters converted to uppercase.
 */
[[nodiscard]] std::string toUpper(std::string s);

// ─── Escaping ────────────────────────────────────────────────────────────────

/**
 * @brief Escapes a string for safe embedding in a JSON value.
 *
 * Handles `"`, `\`, common control characters, and every remaining
 * U+0000–U+001F code point as `\uXXXX`.
 */
[[nodiscard]] std::string escapeJson(const std::string& text);

/**
 * @brief Escapes a string for safe embedding in XML text content or attributes.
 *
 * Handles `<`, `>`, `&`, `"`, `'`, and ASCII control characters (except
 * tab/LF/CR) as `&#N;` numeric character references.
 */
[[nodiscard]] std::string escapeXml(const std::string& text);

/**
 * @brief Escapes a string for safe embedding in HTML text content or attributes.
 *
 * Handles `<`, `>`, `&`, `"`, and `'`.
 */
[[nodiscard]] std::string escapeHtml(const std::string& text);

/**
 * @brief Escapes a string so that special LaTeX characters are printed
 * literally.
 *
 * Handles `\`, `{`, `}`, `$`, `&`, `%`, `#`, `_`, `~`, and `^`.
 */
[[nodiscard]] std::string escapeLatex(const std::string& text);

}  // namespace truss::utils::string
