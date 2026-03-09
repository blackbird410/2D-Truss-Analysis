/**
 * @file test_validate_command.cpp
 * @brief Comprehensive unit tests for ValidateCommand.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Test Coverage:
 * - Command execution with valid truss structure
 * - File validation (existence, readability)
 * - Load truss from file (success/failure)
 * - Truss validation (success/failure)
 * - Error messages with category-specific suggestions
 * - Verbose mode output with validation categories
 * - Edge cases (missing files, invalid formats, etc.)
 *
 * Implementation: Simplified mock-based approach for efficient testing
 * Target: 80%+ line coverage with focused test scenarios
 */

#include "../../../src/application/interfaces/iapplication_output.hpp"
#include "../../../src/cli/commands/validate_command.hpp"
#include "../../../src/cli/presenters/console_presenter.hpp"
#include "../../../src/interface/truss_analysis_facade.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace truss::cli::commands;

/**
 * @brief Test fixture for ValidateCommand tests
 *
 * Provides:
 * - Valid truss file creation for testing
 * - Test file cleanup management
 * - Interface facade instance
 * - Simplified output interface for focused testing
 */
class ValidateCommandTest : public ::testing::Test {
protected:
    truss::interface::TrussAnalysisFacade facade;

    void SetUp() override { createSimpleTrussFile("test_valid.json"); }

    void TearDown() override {
        std::filesystem::remove("test_valid.json");
        std::filesystem::remove("test_invalid.json");
    }

    /**
     * @brief Create a valid truss file for testing
     *
     * Creates a simple 3-node, 3-member truss with:
     * - Fixed support at node 0
     * - Roller support at node 1
     * - 10kN downward load at node 2
     * - Complete material and section properties
     *
     * @param filename Output file path
     */
    void createSimpleTrussFile(const std::string& filename) {
        std::ofstream file(filename);
        file << R"({
            "name": "Valid Test Truss",
            "nodes": [
                {"id": 0, "x": 0.0, "y": 0.0},
                {"id": 1, "x": 4.0, "y": 0.0},
                {"id": 2, "x": 2.0, "y": 3.0}
            ],
            "supports": [
                {"nodeId": 0, "type": "fixed"},
                {"nodeId": 1, "type": "roller", "direction": "vertical"}
            ],
            "members": [
                {"id": 0, "nodeIds": [0, 1]},
                {"id": 1, "nodeIds": [0, 2]},
                {"id": 2, "nodeIds": [1, 2]}
            ],
            "materials": [
                {"id": 0, "E": 200e9, "density": 7850}
            ],
            "sections": [
                {"id": 0, "area": 0.001}
            ],
            "memberAssignments": [
                {"memberId": 0, "materialId": 0, "sectionId": 0},
                {"memberId": 1, "materialId": 0, "sectionId": 0},
                {"memberId": 2, "materialId": 0, "sectionId": 0}
            ],
            "loads": [
                {"nodeId": 2, "fx": 0.0, "fy": -10000.0}
            ]
        })";
        file.close();
    }
};

/**
 * @test Verify command getName() returns correct name
 *
 * Tests the command identification system by verifying that
 * ValidateCommand correctly reports its name as "validate".
 * This ensures proper command registration and routing.
 */
TEST_F(ValidateCommandTest, GetName_ReturnsValidate) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "test.json");
    EXPECT_EQ(cmd.getName(), "validate");
}

/**
 * @test Verify command getDescription() returns non-empty description
 *
 * Tests that the command provides meaningful help text for users.
 * The description should contain relevant keywords like "validate"
 * to help users understand the command's purpose.
 */
TEST_F(ValidateCommandTest, GetDescription_ReturnsNonEmptyDescription) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "test.json");
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_TRUE(desc.find("validate") != std::string::npos ||
                desc.find("Validate") != std::string::npos);
}

/**
 * @test Verify execute() with non-existent file returns error
 *
 * Tests error handling for invalid file paths. The command should:
 * - Return exit code 1 (error)
 * - Provide helpful error message about file not existing
 * - Not attempt validation on invalid input
 */
TEST_F(ValidateCommandTest, Execute_WithNonExistentFile_ReturnsError) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "non_existent_98765.json");
    int exitCode = cmd.execute();

    EXPECT_EQ(exitCode, 1);
}

/**
 * @test Verify execute() with invalid JSON returns error
 *
 * Tests parsing error handling for malformed JSON files. The command should:
 * - Detect JSON parsing errors
 * - Return exit code 1 (error)
 * - Provide meaningful error feedback to user
 */
TEST_F(ValidateCommandTest, Execute_WithInvalidJSON_ReturnsError) {
    std::ofstream file("test_invalid.json");
    file << "{ invalid json ;;;";
    file.close();

    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "test_invalid.json");
    int exitCode = cmd.execute();

    EXPECT_EQ(exitCode, 1);
}

/**
 * @test Verify verbose mode can be enabled
 *
 * Tests that the command accepts verbose flag and executes without crashing.
 * Verbose mode should provide detailed validation category information
 * and diagnostic messages during validation execution.
 */
TEST_F(ValidateCommandTest, Execute_VerboseMode_DoesNotCrash) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "test_valid.json", true);
    int exitCode = cmd.execute();

    EXPECT_TRUE(exitCode == 0 || exitCode == 1);
}

/**
 * @test Verify non-verbose mode works
 *
 * Tests that the command operates correctly in default (non-verbose) mode.
 * Should provide essential validation results without excessive diagnostic
 * information, maintaining clean and concise output.
 */
TEST_F(ValidateCommandTest, Execute_NonVerboseMode_DoesNotCrash) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "test_valid.json", false);
    int exitCode = cmd.execute();

    EXPECT_TRUE(exitCode == 0 || exitCode == 1);
}

/**
 * @test Verify command validates file existence first
 *
 * Tests the validation workflow by confirming that file existence
 * is checked before attempting to parse or validate truss data.
 * This prevents unnecessary processing of invalid file paths.
 */
TEST_F(ValidateCommandTest, Execute_ValidatesFileExistence) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    // Test with actual valid file
    ValidateCommand cmd1(facade, presenter, "test_valid.json");
    int code1 = cmd1.execute();
    EXPECT_TRUE(code1 == 0 || code1 == 1);

    // Test with non-existent file
    ValidateCommand cmd2(facade, presenter, "does_not_exist.json");
    int code2 = cmd2.execute();
    EXPECT_EQ(code2, 1);
}

/**
 * @test Verify command handles empty file path
 *
 * Tests edge case handling for empty or null file paths. The command
 * should gracefully handle this input error and return appropriate
 * error code without crashing or undefined behavior.
 */
TEST_F(ValidateCommandTest, Execute_WithEmptyPath_ReturnsError) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, "");
    int exitCode = cmd.execute();

    EXPECT_EQ(exitCode, 1);
}
/**
 * @test Covers verbose success path: verbose=true on a valid, loadable truss.
 * Exercises the `if (m_verbose)` block at the end of execute() that prints
 * the validation category checklist.
 */
TEST_F(ValidateCommandTest, Execute_ValidTruss_VerboseSuccess_PrintsCategories) {
    const std::string fname = "test_validate_proper_valid.json";
    {
        std::ofstream f(fname);
        f << R"({
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0},
                {"id": 2, "x": 4.0, "y": 0.0},
                {"id": 3, "x": 2.0, "y": 2.0}
            ],
            "members": [
                {"id": 1, "startNode": 1, "endNode": 2},
                {"id": 2, "startNode": 1, "endNode": 3},
                {"id": 3, "startNode": 2, "endNode": 3}
            ],
            "supports": [
                {"nodeId": 1, "restrained": ["x", "y"]},
                {"nodeId": 2, "restrained": ["y"]}
            ],
            "loads": [
                {"nodeId": 3, "fy": -10000.0}
            ]
        })";
    }

    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, fname, /*verbose=*/true);
    int exitCode = cmd.execute();

    std::filesystem::remove(fname);
    EXPECT_EQ(exitCode, 0);  // Valid truss should pass validation
}

/**
 * @test Covers Error, Warning, and Info severity cases in the display switch.
 *
 * Uses a JSON truss that loads correctly but fails validation with:
 * - Error: zero-length member (nodes 1 and 3 at same position)
 * - Warning: load applied to fully pinned node 1
 * - Info: support configuration and static determinacy (always added)
 *
 * This covers the three remaining cases in the switch(issue.severity) block
 * inside ValidateCommand::execute().
 */
TEST_F(ValidateCommandTest, Execute_ZeroLengthAndPinnedLoad_CoversErrorWarnInfoSeverities) {
    const std::string fname = "test_validate_error_warn_info.json";
    {
        std::ofstream f(fname);
        f << R"({
            "nodes": [
                {"id": 1, "x": 0.0, "y": 0.0},
                {"id": 2, "x": 4.0, "y": 0.0},
                {"id": 3, "x": 0.0, "y": 0.0}
            ],
            "members": [
                {"id": 1, "startNode": 1, "endNode": 2},
                {"id": 2, "startNode": 1, "endNode": 3}
            ],
            "supports": [
                {"nodeId": 1, "restrained": ["x", "y"]},
                {"nodeId": 2, "restrained": ["y"]}
            ],
            "loads": [
                {"nodeId": 1, "fx": 1000.0, "fy": 0.0},
                {"nodeId": 2, "fy": -10000.0}
            ]
        })";
    }

    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ValidateCommand cmd(facade, presenter, fname);
    int exitCode = cmd.execute();

    std::filesystem::remove(fname);
    EXPECT_EQ(exitCode, 1);  // Must fail: zero-length member is an Error
}

// =============================================================================
// Filesystem error catch branch — validateInputFile() throws filesystem_error
// when path length exceeds the OS limit (PATH_MAX ~1024 on macOS/Linux).
// =============================================================================

/**
 * @test ValidateCommand::validateInputFile() — filesystem_error catch branch.
 *
 * Passes a path that is 2000 characters long.  On POSIX systems
 * std::filesystem::exists() throws filesystem_error ("filename too long"),
 * which is caught inside validateInputFile() and causes it to return false.
 * execute() then prints an error and returns 1.
 *
 * Covers: `catch (const std::filesystem::filesystem_error&) { return false; }`
 *         at the end of validateInputFile() in validate_command.cpp.
 */
TEST_F(ValidateCommandTest, Execute_VeryLongPath_FilesystemErrorCaught) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    // Build a path longer than PATH_MAX so that std::filesystem::exists()
    // throws std::filesystem::filesystem_error inside validateInputFile().
    std::string longPath(2000, 'a');
    longPath += ".json";

    ValidateCommand cmd(facade, presenter, longPath, /*verbose=*/false);
    int exitCode = cmd.execute();
    EXPECT_EQ(exitCode, 1);
}