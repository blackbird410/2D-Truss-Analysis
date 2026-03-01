/**
 * @file string_utils.hpp
 * @brief String formatting and escaping utilities with no project dependencies.
 * @version 3.0.0
 * @date 2026-03-01
 * @author Neil Taison Rigaud
 */
#pragma once

#include <string>

/**
 * @brief Dependency-free string formatting and output-format escaping utilities.
 *
 * This namespace carries no project-level dependencies and may be safely
 * included at any architectural layer.
 */
namespace truss::utils::string {

// ─── Formatting ──────────────────────────────────────────────────────────────

/**
 * @brief Formats a floating-point number as a string.
 * @param value      Value to format.
 * @param precision  Number of significant digits after the decimal point.
 * @param scientific Use scientific notation when `true`, fixed-point otherwise.
 * @return Formatted string representation of @p value.
 */
[[nodiscard]] std::string formatReal(double value, int precision, bool scientific = false);

/**
 * @brief Returns the current local wall-clock time as a formatted string.
 * @return Local time in the format `YYYY-MM-DD HH:MM:SS`.
 */
[[nodiscard]] std::string formatTimestamp();

// ─── Case conversion ─────────────────────────────────────────────────────────

/**
 * @brief Returns a copy of @p s with all ASCII letters converted to lowercase.
 * @param s Input string.
 * @return Lowercased copy of @p s.
 */
[[nodiscard]] std::string toLower(std::string s);

/**
 * @brief Returns a copy of @p s with all ASCII letters converted to uppercase.
 * @param s Input string.
 * @return Uppercased copy of @p s.
 */
[[nodiscard]] std::string toUpper(std::string s);

// ─── Escaping ────────────────────────────────────────────────────────────────

/**
 * @brief Escapes a string for safe embedding in a JSON string value.
 *
 * Handles `"`, `\`, common control characters (`\b \f \n \r \t`), and every
 * remaining U+0000–U+001F code point as `\uXXXX`.
 *
 * @param text Input string.
 * @return JSON-safe escaped copy of @p text.
 */
[[nodiscard]] std::string escapeJson(const std::string& text);

/**
 * @brief Escapes a string for safe embedding in XML text content or attributes.
 *
 * Handles `<`, `>`, `&`, `"`, `'`, and ASCII control characters (except
 * tab, LF, and CR) as `&#N;` numeric character references.
 *
 * @param text Input string.
 * @return XML-safe escaped copy of @p text.
 */
[[nodiscard]] std::string escapeXml(const std::string& text);

/**
 * @brief Escapes a string for safe embedding in HTML text content or attributes.
 *
 * Handles `<`, `>`, `&`, `"`, and `'`.
 *
 * @param text Input string.
 * @return HTML-safe escaped copy of @p text.
 */
[[nodiscard]] std::string escapeHtml(const std::string& text);

/**
 * @brief Escapes a string so that special LaTeX characters are printed literally.
 *
 * Handles `\`, `{`, `}`, `$`, `&`, `%`, `#`, `_`, `~`, and `^`.
 *
 * @param text Input string.
 * @return LaTeX-safe escaped copy of @p text.
 */
[[nodiscard]] std::string escapeLatex(const std::string& text);

}  // namespace truss::utils::string
