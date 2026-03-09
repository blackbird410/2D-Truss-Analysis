/**
 * @file test_string_utils.cpp
 * @brief Unit tests for truss::utils::string utility functions.
 * @version 3.0.0
 * @date 2026-03-01
 * @author Neil Taison Rigaud
 */

#include "utilities/string_utils.hpp"

#include <gtest/gtest.h>
#include <regex>

using namespace truss::utils::string;

// ─── formatReal ──────────────────────────────────────────────────────────────

TEST(StringUtilsTest, FormatReal_Fixed_DefaultPrecision2) {
    EXPECT_EQ(formatReal(3.14159, 2), "3.14");
}

TEST(StringUtilsTest, FormatReal_Fixed_Precision4) {
    EXPECT_EQ(formatReal(1.23456789, 4), "1.2346");
}

TEST(StringUtilsTest, FormatReal_Fixed_Zero) {
    EXPECT_EQ(formatReal(0.0, 2), "0.00");
}

TEST(StringUtilsTest, FormatReal_Fixed_Negative) {
    EXPECT_EQ(formatReal(-1.5, 1), "-1.5");
}

TEST(StringUtilsTest, FormatReal_Scientific) {
    const std::string result = formatReal(1234.5, 2, true);
    // Should produce something like "1.23e+03"
    EXPECT_NE(result.find('e'), std::string::npos);
}

// ─── formatTimestamp ─────────────────────────────────────────────────────────

TEST(StringUtilsTest, FormatTimestamp_MatchesPattern) {
    const std::string ts = formatTimestamp();
    // Pattern: YYYY-MM-DD HH:MM:SS
    std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(ts, pattern)) << "Got: " << ts;
}

// ─── toLower / toUpper ───────────────────────────────────────────────────────

TEST(StringUtilsTest, ToLower_MixedCase) {
    EXPECT_EQ(toLower("Hello World"), "hello world");
}

TEST(StringUtilsTest, ToLower_AlreadyLower) {
    EXPECT_EQ(toLower("abc"), "abc");
}

TEST(StringUtilsTest, ToLower_AllUpper) {
    EXPECT_EQ(toLower("CSV"), "csv");
}

TEST(StringUtilsTest, ToLower_Empty) {
    EXPECT_EQ(toLower(""), "");
}

TEST(StringUtilsTest, ToUpper_MixedCase) {
    EXPECT_EQ(toUpper("Hello World"), "HELLO WORLD");
}

TEST(StringUtilsTest, ToUpper_Empty) {
    EXPECT_EQ(toUpper(""), "");
}

// ─── escapeJson ──────────────────────────────────────────────────────────────

TEST(StringUtilsTest, EscapeJson_NoSpecialChars) {
    EXPECT_EQ(escapeJson("hello"), "hello");
}

TEST(StringUtilsTest, EscapeJson_DoubleQuote) {
    EXPECT_EQ(escapeJson("say \"hi\""), "say \\\"hi\\\"");
}

TEST(StringUtilsTest, EscapeJson_Backslash) {
    EXPECT_EQ(escapeJson("a\\b"), "a\\\\b");
}

TEST(StringUtilsTest, EscapeJson_Newline) {
    EXPECT_EQ(escapeJson("line1\nline2"), "line1\\nline2");
}

TEST(StringUtilsTest, EscapeJson_Tab) {
    EXPECT_EQ(escapeJson("col1\tcol2"), "col1\\tcol2");
}

TEST(StringUtilsTest, EscapeJson_ControlChar) {
    // U+0001 should become \u0001
    EXPECT_EQ(escapeJson(std::string(1, '\x01')), "\\u0001");
}

// ─── escapeXml ───────────────────────────────────────────────────────────────

TEST(StringUtilsTest, EscapeXml_NoSpecialChars) {
    EXPECT_EQ(escapeXml("hello"), "hello");
}

TEST(StringUtilsTest, EscapeXml_Ampersand) {
    EXPECT_EQ(escapeXml("A & B"), "A &amp; B");
}

TEST(StringUtilsTest, EscapeXml_LessThan) {
    EXPECT_EQ(escapeXml("a < b"), "a &lt; b");
}

TEST(StringUtilsTest, EscapeXml_GreaterThan) {
    EXPECT_EQ(escapeXml("a > b"), "a &gt; b");
}

TEST(StringUtilsTest, EscapeXml_DoubleQuote) {
    EXPECT_EQ(escapeXml("\"value\""), "&quot;value&quot;");
}

TEST(StringUtilsTest, EscapeXml_SingleQuote) {
    EXPECT_EQ(escapeXml("it's"), "it&apos;s");
}

TEST(StringUtilsTest, EscapeXml_AllSpecial) {
    EXPECT_EQ(escapeXml("<a>&'\"</a>"), "&lt;a&gt;&amp;&apos;&quot;&lt;/a&gt;");
}

// ─── escapeHtml ──────────────────────────────────────────────────────────────

TEST(StringUtilsTest, EscapeHtml_NoSpecialChars) {
    EXPECT_EQ(escapeHtml("hello"), "hello");
}

TEST(StringUtilsTest, EscapeHtml_Ampersand) {
    EXPECT_EQ(escapeHtml("A & B"), "A &amp; B");
}

TEST(StringUtilsTest, EscapeHtml_LessThan) {
    EXPECT_EQ(escapeHtml("1 < 2"), "1 &lt; 2");
}

TEST(StringUtilsTest, EscapeHtml_DoubleQuote) {
    EXPECT_EQ(escapeHtml("\"hi\""), "&quot;hi&quot;");
}

TEST(StringUtilsTest, EscapeHtml_SingleQuote) {
    EXPECT_EQ(escapeHtml("it's"), "it&#39;s");
}

// ─── escapeLatex ─────────────────────────────────────────────────────────────

TEST(StringUtilsTest, EscapeLatex_NoSpecialChars) {
    EXPECT_EQ(escapeLatex("hello"), "hello");
}

TEST(StringUtilsTest, EscapeLatex_Backslash) {
    EXPECT_EQ(escapeLatex("a\\b"), "a\\textbackslash{}b");
}

TEST(StringUtilsTest, EscapeLatex_Dollar) {
    EXPECT_EQ(escapeLatex("$100"), "\\$100");
}

TEST(StringUtilsTest, EscapeLatex_Percent) {
    EXPECT_EQ(escapeLatex("50%"), "50\\%");
}

TEST(StringUtilsTest, EscapeLatex_Ampersand) {
    EXPECT_EQ(escapeLatex("A & B"), "A \\& B");
}

TEST(StringUtilsTest, EscapeLatex_Tilde) {
    EXPECT_EQ(escapeLatex("~approx"), "\\textasciitilde{}approx");
}

TEST(StringUtilsTest, EscapeLatex_Underscore) {
    EXPECT_EQ(escapeLatex("x_1"), "x\\_1");
}

TEST(StringUtilsTest, EscapeLatex_Caret) {
    EXPECT_EQ(escapeLatex("x^2"), "x\\textasciicircum{}2");
}
// ─── Additional escapeJson branches ──────────────────────────────────────────

TEST(StringUtilsTest, EscapeJson_Backspace) {
    EXPECT_EQ(escapeJson(std::string(1, '\b')), "\\b");
}

TEST(StringUtilsTest, EscapeJson_FormFeed) {
    EXPECT_EQ(escapeJson(std::string(1, '\f')), "\\f");
}

TEST(StringUtilsTest, EscapeJson_CarriageReturn) {
    EXPECT_EQ(escapeJson(std::string(1, '\r')), "\\r");
}

// ─── Additional escapeXml branch ─────────────────────────────────────────────

TEST(StringUtilsTest, EscapeXml_ControlChar) {
    // U+0001 should become &#1;
    EXPECT_EQ(escapeXml(std::string(1, '\x01')), "&#1;");
}

// ─── Additional escapeHtml branch ────────────────────────────────────────────

TEST(StringUtilsTest, EscapeHtml_GreaterThan) {
    EXPECT_EQ(escapeHtml("2 > 1"), "2 &gt; 1");
}

// ─── Additional escapeLatex branches ─────────────────────────────────────────

TEST(StringUtilsTest, EscapeLatex_CurlyBraces) {
    EXPECT_EQ(escapeLatex("{x}"), "\\{x\\}");
}

TEST(StringUtilsTest, EscapeLatex_Hash) {
    EXPECT_EQ(escapeLatex("#1"), "\\#1");
}