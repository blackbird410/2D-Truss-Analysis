/**
 * @file test_string_utils_branches.cpp
 * @brief Branch coverage tests for string utility escape functions.
 * @version 3.0.0
 *
 * Covers escape function switch cases not exercised by the primary
 * test_string_utils.cpp:
 *   escapeJson   – \b, \f, \r cases
 *   escapeXml    – control-character default branch (c < 0x20 not \t\n\r)
 *   escapeHtml   – '>' case
 *   escapeLatex  – '{', '}', '#' cases
 */

#include "utilities/string_utils.hpp"

#include <gtest/gtest.h>

using namespace truss::utils::string;

// ---------------------------------------------------------------------------
// escapeJson – missing switch cases
// ---------------------------------------------------------------------------

TEST(StringUtilsBranchTest, EscapeJson_Backspace_IsEscaped) {
    EXPECT_EQ(escapeJson("\b"), "\\b");
}

TEST(StringUtilsBranchTest, EscapeJson_FormFeed_IsEscaped) {
    EXPECT_EQ(escapeJson("\f"), "\\f");
}

TEST(StringUtilsBranchTest, EscapeJson_CarriageReturn_IsEscaped) {
    EXPECT_EQ(escapeJson("\r"), "\\r");
}

// Control characters below 0x20 that are not one of the named cases
// should produce a \uXXXX unicode escape.
TEST(StringUtilsBranchTest, EscapeJson_OtherControlChar_ProducesUnicodeEscape) {
    // 0x01 (SOH) → \u0001
    std::string input{'\x01'};
    EXPECT_EQ(escapeJson(input), "\\u0001");
}

// Multiple mixed characters in a single call
TEST(StringUtilsBranchTest, EscapeJson_Mixed_AllCases) {
    std::string input = "a\"\\\b\f\n\r\t";
    std::string expected = "a\\\"\\\\\\b\\f\\n\\r\\t";
    EXPECT_EQ(escapeJson(input), expected);
}

// ---------------------------------------------------------------------------
// escapeXml – control-character default branch
// ---------------------------------------------------------------------------

// Characters with value < 0x20 that are NOT \t (0x09), \n (0x0A), \r (0x0D)
// should be encoded as &#N;
TEST(StringUtilsBranchTest, EscapeXml_ControlChar_ProducesNumericEntity) {
    std::string input{'\x01'};  // SOH
    EXPECT_EQ(escapeXml(input), "&#1;");
}

TEST(StringUtilsBranchTest, EscapeXml_ControlChar_0x1F_ProducesNumericEntity) {
    std::string input{'\x1F'};  // US (unit separator)
    EXPECT_EQ(escapeXml(input), "&#31;");
}

// Tabs, newlines and CRs should pass through unchanged.
TEST(StringUtilsBranchTest, EscapeXml_WhitespaceControls_PassThrough) {
    EXPECT_EQ(escapeXml("\t"), "\t");
    EXPECT_EQ(escapeXml("\n"), "\n");
    EXPECT_EQ(escapeXml("\r"), "\r");
}

// ---------------------------------------------------------------------------
// escapeHtml – GreaterThan case
// ---------------------------------------------------------------------------

TEST(StringUtilsBranchTest, EscapeHtml_GreaterThan_IsEscaped) {
    EXPECT_EQ(escapeHtml(">"), "&gt;");
}

TEST(StringUtilsBranchTest, EscapeHtml_AllSpecial_Roundtrip) {
    EXPECT_EQ(escapeHtml("<>&\"'"), "&lt;&gt;&amp;&quot;&#39;");
}

// ---------------------------------------------------------------------------
// escapeLatex – {, }, # cases
// ---------------------------------------------------------------------------

TEST(StringUtilsBranchTest, EscapeLatex_OpenBrace_IsEscaped) {
    EXPECT_EQ(escapeLatex("{"), "\\{");
}

TEST(StringUtilsBranchTest, EscapeLatex_CloseBrace_IsEscaped) {
    EXPECT_EQ(escapeLatex("}"), "\\}");
}

TEST(StringUtilsBranchTest, EscapeLatex_Hash_IsEscaped) {
    EXPECT_EQ(escapeLatex("#"), "\\#");
}

TEST(StringUtilsBranchTest, EscapeLatex_AllSpecial_Roundtrip) {
    // Covers all 10 switch cases: \, {, }, $, &, %, #, _, ~, ^
    std::string input = "\\{}$&%#_~^";
    std::string expected =
        "\\textbackslash{}\\{\\}\\$\\&\\%\\#\\_"
        "\\textasciitilde{}\\textasciicircum{}";
    EXPECT_EQ(escapeLatex(input), expected);
}
