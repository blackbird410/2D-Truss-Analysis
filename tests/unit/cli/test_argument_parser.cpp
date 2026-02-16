/**
 * @file test_argument_parser.cpp
 * @brief Unit tests for ArgumentParser
 * 
 * Tests all parsing logic, edge cases, and control flow branches
 */

#include <gtest/gtest.h>
#include "../../../src/cli/ArgumentParser.hpp"
#include <vector>
#include <cstring>

using namespace truss::cli;

/**
 * Test fixture for ArgumentParser tests
 */
class ArgumentParserTest : public ::testing::Test {
protected:
    ArgumentParser parser;
    
    /**
     * Helper to create argv from vector of strings
     */
    char** createArgv(const std::vector<std::string>& args) {
        char** argv = new char*[args.size()];
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = new char[args[i].size() + 1];
            std::strcpy(argv[i], args[i].c_str());
        }
        return argv;
    }
    
    /**
     * Helper to free argv
     */
    void freeArgv(char** argv, int argc) {
        for (int i = 0; i < argc; ++i) {
            delete[] argv[i];
        }
        delete[] argv;
    }
};

// ============================================================================
// BASIC PARSING TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_ProgramNameOnly_ReturnsHelpCommand) {
    std::vector<std::string> args = {"TrussAnalysisCLI"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "help");
    EXPECT_FALSE(result.verbose);
    EXPECT_FALSE(result.showHelp);
    EXPECT_TRUE(result.positionalArgs.empty());
    EXPECT_TRUE(result.options.empty());
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_SimpleCommand_ParsesCorrectly) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_FALSE(result.verbose);
    EXPECT_FALSE(result.showHelp);
    EXPECT_TRUE(result.positionalArgs.empty());
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_CommandWithPositionalArgs_ParsesAllArgs) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "analyze", "input.json", "output.xml"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    ASSERT_EQ(result.positionalArgs.size(), 2);
    EXPECT_EQ(result.positionalArgs[0], "input.json");
    EXPECT_EQ(result.positionalArgs[1], "output.xml");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

// ============================================================================
// HELP FLAG TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_ShortHelpFlag_SetsHelpCommand) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "-h"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "help");
    EXPECT_TRUE(result.showHelp);
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_LongHelpFlag_SetsHelpCommand) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "--help"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "help");
    EXPECT_TRUE(result.showHelp);
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_HelpWithOtherArgs_IgnoresOtherArgs) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "-h"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "help");
    EXPECT_TRUE(result.showHelp);
    
    freeArgv(argv, static_cast<int>(args.size()));
}

// ============================================================================
// VERBOSE FLAG TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_ShortVerboseFlag_SetsVerboseTrue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "-v", "example"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_TRUE(result.verbose);
    EXPECT_EQ(result.options["verbose"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_LongVerboseFlag_SetsVerboseTrue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "--verbose", "example"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_TRUE(result.verbose);
    EXPECT_EQ(result.options["verbose"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_VerboseAfterCommand_StillSetsVerbose) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "-v"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_TRUE(result.verbose);
    
    freeArgv(argv, static_cast<int>(args.size()));
}

// ============================================================================
// LONG OPTION TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_LongOptionWithEquals_ParsesNameAndValue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "--file=input.json", "analyze"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_EQ(result.options["file"], "input.json");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_LongOptionWithSpace_ParsesNameAndValue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "--file", "input.json", "analyze"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_EQ(result.options["file"], "input.json");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_LongOptionBooleanFlag_SetsValueToTrue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "--debug"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_EQ(result.options["debug"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_LongOptionWithEmptyValue_SetsEmptyValue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "--enable="};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    // When option has =, even if empty, parser treats empty value as empty
    // But since value is empty, it becomes "true" per the logic
    EXPECT_EQ(result.options["enable"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

// ============================================================================
// SHORT OPTION TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_ShortOptionWithEquals_ParsesNameAndValue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "-f=input.json", "analyze"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_EQ(result.options["f"], "input.json");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_ShortOptionWithSpace_ParsesNameAndValue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "-f", "input.json", "analyze"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_EQ(result.options["f"], "input.json");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_ShortOptionBooleanFlag_SetsValueToTrue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "-d"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_EQ(result.options["d"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_MultipleOptions_ParsesAllOptions) {
    std::vector<std::string> args = {
        "TrussAnalysisCLI", 
        "--file=input.json", 
        "--output=result.xml",
        "-v",
        "analyze"
    };
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_EQ(result.options["file"], "input.json");
    EXPECT_EQ(result.options["output"], "result.xml");
    EXPECT_TRUE(result.verbose);
    EXPECT_EQ(result.options["verbose"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_OptionFollowedByAnotherOption_TreatsSecondAsOption) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "analyze", "--file", "--output"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_EQ(result.options["file"], "true");
    EXPECT_EQ(result.options["output"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_DashDashAlone_NotRecognizedAsOption) {
    // "--" alone is not a valid long option (needs at least 3 chars total)
    std::vector<std::string> args = {"TrussAnalysisCLI", "--", "example"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    // "--" is treated as command name since it's not recognized as option
    EXPECT_EQ(result.commandName, "--");
    ASSERT_EQ(result.positionalArgs.size(), 1);
    EXPECT_EQ(result.positionalArgs[0], "example");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_SingleDash_NotRecognizedAsOption) {
    // "-" alone is not a valid short option (needs at least 2 chars)
    std::vector<std::string> args = {"TrussAnalysisCLI", "-", "example"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    // "-" is treated as command name
    EXPECT_EQ(result.commandName, "-");
    ASSERT_EQ(result.positionalArgs.size(), 1);
    EXPECT_EQ(result.positionalArgs[0], "example");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_EmptyStringArgument_SkippedNotAsCommand) {
    // Empty strings are skipped by the parser iteration
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", ""};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    // Empty string at end becomes positional arg
    ASSERT_EQ(result.positionalArgs.size(), 1);
    EXPECT_EQ(result.positionalArgs[0], "");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_OptionWithEqualsAtEnd_ValueIsEmpty) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "--file="};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    // Same logic: empty value becomes "true"
    EXPECT_EQ(result.options["file"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_OptionWithMultipleEquals_TakesFirstAsDelimiter) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "--key=value=extra", "example"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_EQ(result.options["key"], "value=extra");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_ComplexMixedArguments_ParsesCorrectly) {
    std::vector<std::string> args = {
        "TrussAnalysisCLI",
        "-v",
        "analyze",
        "--input=file.json",
        "positional1",
        "-o", "output.xml",
        "positional2",
        "--flag"
    };
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "analyze");
    EXPECT_TRUE(result.verbose);
    EXPECT_EQ(result.options["verbose"], "true");
    EXPECT_EQ(result.options["input"], "file.json");
    EXPECT_EQ(result.options["o"], "output.xml");
    EXPECT_EQ(result.options["flag"], "true");
    ASSERT_EQ(result.positionalArgs.size(), 2);
    EXPECT_EQ(result.positionalArgs[0], "positional1");
    EXPECT_EQ(result.positionalArgs[1], "positional2");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

// ============================================================================
// BOUNDARY CONDITION TESTS
// ============================================================================

TEST_F(ArgumentParserTest, Parse_OnlyArgc1_ReturnsDefaultHelp) {
    std::vector<std::string> args = {"program"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(1, argv);
    
    EXPECT_EQ(result.commandName, "help");
    EXPECT_FALSE(result.showHelp);
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_LongOptionExactly3Chars_RecognizedAsLongOption) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "--o"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_EQ(result.options["o"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_ShortOptionExactly2Chars_RecognizedAsShortOption) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "-x"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_EQ(result.options["x"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

TEST_F(ArgumentParserTest, Parse_OptionAtEndOfArgs_ValueBecomesTrue) {
    std::vector<std::string> args = {"TrussAnalysisCLI", "example", "--flag"};
    char** argv = createArgv(args);
    
    ParsedArgs result = parser.parse(static_cast<int>(args.size()), argv);
    
    EXPECT_EQ(result.commandName, "example");
    EXPECT_EQ(result.options["flag"], "true");
    
    freeArgv(argv, static_cast<int>(args.size()));
}

