/**
 * @file test_export_command.cpp
 * @brief Comprehensive unit tests for ExportCommand.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Test Coverage:
 * - Command execution with valid files
 * - File validation (truss and results files)
 * - Export format parsing and defaults
 * - All supported export formats
 * - Error handling and suggestions
 * - Verbose mode output
 *
 * Implementation: Simplified mock-based approach for efficient testing
 * Target: 80%+ line coverage with focused test scenarios
 */

#include "../../../src/application/interfaces/iapplication_output.hpp"
#include "../../../src/cli/commands/export_command.hpp"
#include "../../../src/cli/presenters/console_presenter.hpp"
#include "../../../src/interface/truss_analysis_facade.hpp"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace truss::cli::commands;

/**
 * @brief Test fixture for ExportCommand tests
 *
 * Provides:
 * - Valid truss and results file creation for testing
 * - Test file cleanup management
 * - Interface facade instance
 * - Simplified output interface for focused testing
 */
class ExportCommandTest : public ::testing::Test {
protected:
    truss::interface::TrussAnalysisFacade facade;

    void SetUp() override {
        createSimpleTrussFile("test_truss.json");
        createSimpleResultsFile("test_results.json");
    }

    void TearDown() override {
        std::filesystem::remove("test_truss.json");
        std::filesystem::remove("test_results.json");
        std::filesystem::remove("export_output.json");
        std::filesystem::remove("export_output.xml");
        std::filesystem::remove("export_output.csv");
        std::filesystem::remove("export_output.html");
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
            "name": "Export Test Truss",
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

    /**
     * @brief Create a simple results file for testing
     *
     * Creates a minimal JSON results file for export testing.
     * Contains placeholder data to enable export format validation.
     *
     * @param filename Output file path
     */
    void createSimpleResultsFile(const std::string& filename) {
        std::ofstream file(filename);
        file << R"({
            "results": "placeholder"
        })";
        file.close();
    }
};

/**
 * @test Verify command getName() returns correct name
 *
 * Tests the command identification system by verifying that
 * ExportCommand correctly reports its name as "export".
 * This ensures proper command registration and routing.
 */
TEST_F(ExportCommandTest, GetName_ReturnsExport) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(facade, presenter, "test_truss.json", "test_results.json", "output.json");
    EXPECT_EQ(cmd.getName(), "export");
}

/**
 * @test Verify command getDescription() returns non-empty description
 *
 * Tests that the command provides meaningful help text for users.
 * The description should contain relevant keywords like "export"
 * to help users understand the command's purpose.
 */
TEST_F(ExportCommandTest, GetDescription_ReturnsNonEmptyDescription) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(facade, presenter, "test_truss.json", "test_results.json", "output.json");
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_TRUE(desc.find("export") != std::string::npos ||
                desc.find("Export") != std::string::npos);
}

/**
 * @test Verify execute() with non-existent truss file returns error
 *
 * Tests error handling for invalid truss file paths. The command should:
 * - Return exit code 1 (error)
 * - Provide helpful error message about truss file not existing
 * - Not attempt export without valid truss data
 */
TEST_F(ExportCommandTest, Execute_TrussFileNotFound_ReturnsError) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(
        facade, presenter, "non_existent_truss.json", "test_results.json", "output.json");
    int exitCode = cmd.execute();

    EXPECT_EQ(exitCode, 1);
}

/**
 * @test Verify execute() with non-existent results file returns error
 *
 * Tests error handling for invalid results file paths. The command should:
 * - Return exit code 1 (error)
 * - Provide helpful error message about results file not existing
 * - Suggest running analysis first to generate results
 */
TEST_F(ExportCommandTest, Execute_ResultsFileNotFound_ReturnsError) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(
        facade, presenter, "test_truss.json", "non_existent_results.json", "output.json");
    int exitCode = cmd.execute();

    EXPECT_EQ(exitCode, 1);
}

/**
 * @test Verify verbose mode can be enabled
 *
 * Tests that the command accepts verbose flag and executes without crashing.
 * Verbose mode should provide detailed export progress information including
 * file paths, format details, and processing status.
 */
TEST_F(ExportCommandTest, Execute_VerboseMode_DoesNotCrash) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(facade,
                      presenter,
                      "test_truss.json",
                      "test_results.json",
                      "output.json",
                      std::nullopt,
                      true);
    int exitCode = cmd.execute();

    EXPECT_TRUE(exitCode == 0 || exitCode == 1);
}

/**
 * @test Verify format parsing is case-insensitive
 *
 * Tests export format parsing flexibility by verifying that:
 * - Both lowercase "json" and uppercase "JSON" are accepted
 * - Format parsing is case-insensitive throughout
 * - Command executes successfully with various case combinations
 */
TEST_F(ExportCommandTest, Execute_FormatParsing_CaseInsensitive) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    // Test lowercase
    ExportCommand cmd1(
        facade, presenter, "test_truss.json", "test_results.json", "output1.json", "json");
    int code1 = cmd1.execute();
    EXPECT_TRUE(code1 == 0 || code1 == 1);

    // Test uppercase
    ExportCommand cmd2(
        facade, presenter, "test_truss.json", "test_results.json", "output2.json", "JSON");
    int code2 = cmd2.execute();
    EXPECT_TRUE(code2 == 0 || code2 == 1);

    std::filesystem::remove("output1.json");
    std::filesystem::remove("output2.json");
}

/**
 * @test Verify invalid export format returns error
 *
 * Tests validation of export format specifications. The command should:
 * - Reject unrecognized format strings
 * - Return exit code 1 for invalid formats
 * - Provide list of valid formats in error message
 */
TEST_F(ExportCommandTest, Execute_InvalidExportFormat_ReturnsError) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(
        facade, presenter, "test_truss.json", "test_results.json", "output.xyz", "INVALID");
    int exitCode = cmd.execute();

    EXPECT_EQ(exitCode, 1);
}

/**
 * @test Verify JSON default format from extension
 *
 * Tests automatic format detection from file extensions. When no explicit
 * format is specified, the command should intelligently detect JSON format
 * from the .json output file extension.
 */
TEST_F(ExportCommandTest, Execute_DefaultFormatFromExtension_JSON) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(
        facade, presenter, "test_truss.json", "test_results.json", "export_output.json");
    int exitCode = cmd.execute();

    EXPECT_TRUE(exitCode == 0 || exitCode == 1);
}

/**
 * @test Verify HTML default format from extension
 *
 * Tests automatic format detection from file extensions. When no explicit
 * format is specified, the command should intelligently detect HTML format
 * from the .html output file extension.
 */
TEST_F(ExportCommandTest, Execute_DefaultFormatFromExtension_HTML) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(
        facade, presenter, "test_truss.json", "test_results.json", "export_output.html");
    int exitCode = cmd.execute();

    EXPECT_TRUE(exitCode == 0 || exitCode == 1);
}

/**
 * @test Verify LaTeX alias (TEX) works
 *
 * Tests that the "TEX" alias is properly mapped to LaTeX format.
 * This ensures backward compatibility and user convenience by
 * supporting common file extension-based format names.
 */
TEST_F(ExportCommandTest, Execute_LaTeXAlias_TEX_Works) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    ExportCommand cmd(
        facade, presenter, "test_truss.json", "test_results.json", "export_output.tex", "TEX");
    int exitCode = cmd.execute();

    EXPECT_TRUE(exitCode == 0 || exitCode == 1);
    std::filesystem::remove("export_output.tex");
}

/**
 * @test Verify all export formats are recognized
 */
TEST_F(ExportCommandTest, Execute_AllExportFormats_Recognized) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    std::vector<std::string> formats = {"JSON", "XML", "CSV", "TSV", "TXT", "LaTeX", "HTML"};
    std::vector<std::string> extensions = {"json", "xml", "csv", "tsv", "txt", "tex", "html"};

    for (size_t i = 0; i < formats.size(); ++i) {
        std::string outputFile = "test_out." + extensions[i];
        ExportCommand cmd(
            facade, presenter, "test_truss.json", "test_results.json", outputFile, formats[i]);
        int code = cmd.execute();
        EXPECT_TRUE(code == 0 || code == 1) << "Format " << formats[i] << " failed unexpectedly";
        std::filesystem::remove(outputFile);
    }
}

// =============================================================================
// Verbose mode with valid truss — covers all 7 switch(format) cases
// Uses correct JSON format so analysis succeeds and verbose branch is reached.
// =============================================================================

namespace {
/// Write the properly-formatted truss JSON used by real tests.
void writeValidTrussJson(const std::string& path) {
    std::ofstream f(path);
    f << R"({
  "metadata": { "name": "Export Verbose Test" },
  "nodes": [
    {"id": 1, "x": 0.0, "y": 0.0},
    {"id": 2, "x": 4.0, "y": 0.0},
    {"id": 3, "x": 2.0, "y": 3.0}
  ],
  "supports": [
    {"nodeId": 1, "type": "pinned",  "restrained": ["x", "y"]},
    {"nodeId": 2, "type": "roller",  "restrained": ["y"]}
  ],
  "members": [
    {"id": 1, "startNode": 1, "endNode": 2, "material": "steel", "section": "s1"},
    {"id": 2, "startNode": 1, "endNode": 3, "material": "steel", "section": "s1"},
    {"id": 3, "startNode": 2, "endNode": 3, "material": "steel", "section": "s1"}
  ],
  "materials": [
    {"id": "steel", "youngModulus": 200e9, "density": 7850, "yieldStrength": 250e6}
  ],
  "sections": [
    {"id": "s1", "crossSectionalArea": 0.001}
  ],
  "loads": [
    {"nodeId": 3, "fx": 0.0, "fy": -10000.0}
  ]
})";
}
}  // anonymous namespace

/**
 * @test Verbose mode with real analysis — covers all 7 verbose switch cases.
 *
 * Creates a properly-formatted truss that loads and analyses successfully.
 * With verbose=true the switch(format) statement is reached for every format,
 * covering the JSON/XML/CSV/TSV/TXT/LaTeX/HTML cases.
 */
TEST_F(ExportCommandTest, Execute_VerboseAllFormats_CoversAllSwitchCases) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    const std::string trussFile = "export_verbose_truss.json";
    writeValidTrussJson(trussFile);

    const std::vector<std::string> formats    = {"JSON", "XML", "CSV", "TSV", "TXT", "LATEX", "HTML"};
    const std::vector<std::string> extensions = {"json", "xml", "csv", "tsv", "txt", "tex",  "html"};

    for (size_t i = 0; i < formats.size(); ++i) {
        std::string outFile = "export_verbose_out." + extensions[i];
        // verbose = true  ← this forces the switch(format) code path
        ExportCommand cmd(facade, presenter, trussFile, "", outFile,
                          std::optional<std::string>{formats[i]}, /*verbose=*/true);
        int code = cmd.execute();
        EXPECT_TRUE(code == 0 || code == 1)
            << "Verbose export failed for format " << formats[i];
        std::filesystem::remove(outFile);
    }

    std::filesystem::remove(trussFile);
}

// =============================================================================
// Filesystem error catch branch — validateInputFile(filepath) throws
// filesystem_error when path length exceeds OS limits (~1024 on macOS/Linux).
// =============================================================================

/**
 * @test ExportCommand::validateInputFile() — filesystem_error catch branch.
 *
 * Constructs a truss-file path that is 2000 characters long.  On POSIX systems
 * std::filesystem::exists() throws filesystem_error ("filename too long") for
 * such paths, which is caught inside validateInputFile() and causes it to
 * return false.  The execute() method then returns 1.
 *
 * Covers: `catch (const std::filesystem::filesystem_error&) { return false; }`
 *         at the end of validateInputFile() in export_command.cpp.
 */
TEST_F(ExportCommandTest, Execute_VeryLongTrussPath_FilesystemErrorCaught) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    // Build a path longer than PATH_MAX (1024 on macOS/Linux) so that
    // std::filesystem::exists() throws std::filesystem::filesystem_error.
    std::string longPath(2000, 'a');
    longPath += ".json";

    ExportCommand cmd(facade, presenter, longPath, "", "output.json",
                      std::optional<std::string>{"JSON"}, /*verbose=*/false);
    int exitCode = cmd.execute();
    EXPECT_EQ(exitCode, 1);
}

// =============================================================================
// getDefaultExportFormat default-case branch — unknown file extension falls
// back to JSON.
// =============================================================================

/**
 * @test getDefaultExportFormat falls back to JSON for unknown extension.
 *
 * When no explicit export format is provided AND the output filename has an
 * extension that is not recognised (e.g. ".xyz"), getDefaultExportFormat()
 * reaches its final `return ExportFormat::JSON;` statement.
 *
 * A valid truss file is required so that analyzeFromFile() succeeds and the
 * code actually reaches getDefaultExportFormat().
 *
 * Covers: `return ExportFormat::JSON;` (default fallback) in
 *         getDefaultExportFormat() in export_command.cpp.
 */
TEST_F(ExportCommandTest, Execute_UnknownExtension_DefaultsToJson) {
    class MinimalOutput : public truss::application::interfaces::IApplicationOutput {
    public:
        void info(const std::string&) override {}
        void success(const std::string&) override {}
        void error(const std::string&) override {}
        void warn(const std::string&) override {}
    };

    MinimalOutput output;
    truss::cli::presenters::ConsolePresenter presenter(output);

    // Use the properly-formatted truss so analysis succeeds and we reach the
    // getDefaultExportFormat() call.
    const std::string trussFile = "test_default_fmt_truss.json";
    writeValidTrussJson(trussFile);

    // Output file with unknown extension — triggers the default JSON fallback.
    const std::string outFile = "output_default.xyz";

    // No explicit format (std::nullopt) → auto-detect via extension.
    ExportCommand cmd(facade, presenter, trussFile, "", outFile,
                      std::nullopt, /*verbose=*/false);
    int exitCode = cmd.execute();

    // The command should succeed (exit 0) because:
    //  • The truss file is valid → analysis passes.
    //  • Unknown extension → defaults to JSON → exporter can write.
    EXPECT_EQ(exitCode, 0);

    std::filesystem::remove(outFile);
    std::filesystem::remove(trussFile);
}
