/**
 * @file test_export_command_mock.cpp
 * @brief Mock-based branch coverage tests for ExportCommand.
 * @version 3.0.0
 *
 * Uses NiceMock<MockTrussAnalysisFacade> to reach branches that
 * are unreachable with a real facade whose analysis always fails.
 *
 * Covered branches:
 *   - verbose format-name switch: all 7 cases (JSON/XML/CSV/TSV/TXT/LaTeX/HTML)
 *   - verbose post-analysis-success displaySuccess
 *   - analysisResult failure path (!analysisResult)
 *   - exportResults failure path (!facade.exportResults(...))
 *   - validateInputFile() directory branch (is_regular_file == false)
 *   - getDefaultExportFormat() extension fallback
 */

#include "application/interfaces/iapplication_output.hpp"
#include "cli/commands/export_command.hpp"
#include "cli/presenters/console_presenter.hpp"
#include "interface/itruss_analysis_facade.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

using namespace truss::cli::commands;
using namespace truss::interface;
using ::testing::_;
using ::testing::An;
using ::testing::NiceMock;
using ::testing::Return;

// ---------------------------------------------------------------------------
// Minimal sink for ConsolePresenter output
// ---------------------------------------------------------------------------

class SilentOutput : public truss::application::interfaces::IApplicationOutput {
public:
    void info(const std::string&) override {}
    void success(const std::string&) override {}
    void error(const std::string&) override {}
    void warn(const std::string&) override {}
};

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class ExportCommandMockTest : public ::testing::Test {
protected:
    NiceMock<truss::test::MockTrussAnalysisFacade> mockFacade;
    SilentOutput silentOut;
    truss::cli::presenters::ConsolePresenter presenter{silentOut};

    std::string trussFile{"ec_mock_truss.json"};
    std::string resultsFile{"ec_mock_results.json"};
    std::string outputFile{"ec_mock_output.json"};

    void SetUp() override {
        // Write a minimal placeholder file so validateInputFile() passes.
        std::ofstream f(trussFile);
        f << "{}";
        f.close();

        // Set up default behaviour: analysis succeeds, export succeeds.
        ON_CALL(mockFacade, analyzeFromFile(_, _))
            .WillByDefault(Return(AnalysisWorkflowResult::Success(0, 0)));
        // Disambiguate the 4-arg exportResults(handle, ExportFormat, path, options)
        // overload from the 3-arg overload by typing the second argument.
        ON_CALL(mockFacade,
                exportResults(_, An<truss::ExportFormat>(), _, _))
            .WillByDefault(Return(true));
    }

    void TearDown() override {
        std::filesystem::remove(trussFile);
        std::filesystem::remove(resultsFile);
        std::filesystem::remove(outputFile);
        for (const auto& ext : {".xml", ".csv", ".tsv", ".txt", ".tex", ".html"}) {
            std::filesystem::remove("ec_mock_output" + std::string(ext));
        }
    }
};

// ---------------------------------------------------------------------------
// Analysis failure path
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_AnalysisFailure_ReturnsOne) {
    ON_CALL(mockFacade, analyzeFromFile(_, _))
        .WillByDefault(Return(AnalysisWorkflowResult::Failure("simulated error")));

    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile, outputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Export results failure path
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_ExportResultsFailure_ReturnsOne) {
    ON_CALL(mockFacade, exportResults(_, An<truss::ExportFormat>(), _, _))
        .WillByDefault(Return(false));

    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile, outputFile,
                      std::optional<std::string>{"JSON"});
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Directory input → validateInputFile returns false
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_DirectoryAsInput_ReturnsOne) {
    namespace fs = std::filesystem;
    std::string dirPath = "ec_mock_tmpdir";
    fs::create_directory(dirPath);

    ExportCommand cmd(mockFacade, presenter, dirPath, resultsFile, outputFile);
    int rc = cmd.execute();

    fs::remove(dirPath);
    EXPECT_EQ(rc, 1);
}

// ---------------------------------------------------------------------------
// Verbose mode + format-name switch – all 7 cases
// Each test passes exact format string and a verbose flag, verifies success.
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_VerboseMode_JsonFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.json",
                      std::optional<std::string>{"JSON"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_VerboseMode_XmlFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.xml",
                      std::optional<std::string>{"XML"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_VerboseMode_CsvFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.csv",
                      std::optional<std::string>{"CSV"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_VerboseMode_TsvFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.tsv",
                      std::optional<std::string>{"TSV"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_VerboseMode_TxtFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.txt",
                      std::optional<std::string>{"TXT"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_VerboseMode_LatexFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.tex",
                      std::optional<std::string>{"LATEX"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_VerboseMode_HtmlFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.html",
                      std::optional<std::string>{"HTML"}, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// Non-verbose success path (no format string → auto-detect from extension)
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_NonVerboseAutoFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.xml");
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// getDefaultExportFormat – rare extensions (TSV, TXT, TEX, HTML + fallback)
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_DefaultFormat_TsvExtension_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.tsv");
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_DefaultFormat_TxtExtension_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.txt");
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_DefaultFormat_TexExtension_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.tex");
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_DefaultFormat_HtmlExtension_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.html");
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest,
       Execute_DefaultFormat_UnknownExtension_FallsBackToJson) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.xyz");
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// Verbose mode with no explicit format (getDefaultExportFormat is also hit)
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_VerboseMode_AutoFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.csv", std::nullopt, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// parseExportFormat: "TEX" alias for LaTeX
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_TexAliasFormat_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.tex",
                      std::optional<std::string>{"TEX"}, /*verbose=*/false);
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// parseExportFormat: unrecognized format string → returns 1
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_InvalidFormatString_ReturnsOne) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.dat",
                      std::optional<std::string>{"UNKNOWN_FMT"}, /*verbose=*/false);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// validateInputFile: non-existent input file → returns 1
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_NonExistentTrussFile_ReturnsOne) {
    ExportCommand cmd(mockFacade, presenter, "/nonexistent/path/truss.json",
                      resultsFile, "ec_mock_output.csv");
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// parseExportFormat: "TSV" and "TXT" explicit format strings
// ---------------------------------------------------------------------------

TEST_F(ExportCommandMockTest, Execute_TsvFormatString_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.tsv",
                      std::optional<std::string>{"TSV"}, /*verbose=*/false);
    EXPECT_EQ(cmd.execute(), 0);
}

TEST_F(ExportCommandMockTest, Execute_TxtFormatString_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.txt",
                      std::optional<std::string>{"TXT"}, /*verbose=*/false);
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// getDefaultExportFormat: .json extension → covers line 193
// ---------------------------------------------------------------------------

/**
 * @test Covers export_command.cpp line 193: getDefaultExportFormat returns
 *       ExportFormat::JSON when the output filepath has a .json extension.
 */
TEST_F(ExportCommandMockTest,
       Execute_DefaultFormat_JsonExtension_ReturnsZero) {
    ExportCommand cmd(mockFacade, presenter, trussFile, resultsFile,
                      "ec_mock_output.json");  // .json extension, no format
    EXPECT_EQ(cmd.execute(), 0);
}
