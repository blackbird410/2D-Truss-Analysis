/**
 * @file test_validate_command_mock.cpp
 * @brief Mock-based branch coverage tests for ValidateCommand.
 * @version 3.0.0
 *
 * Uses NiceMock<MockTrussAnalysisFacade> to exercise branches that a real
 * facade cannot reach (validation success/fail, issue categories, verbosity,
 * all 4 severity labels, directory input).
 *
 * Covered branches:
 *   - validateFromFile returns valid → success path
 *   - validateFromFile returns invalid → failure path
 *   - verbose success path (8 category lines)
 *   - severity switch: Fatal, Error, Warning, Info
 *   - suggestion blocks: hasSupport, hasMember, hasLoad, general
 *   - validateInputFile() directory branch
 */

#include "application/interfaces/iapplication_output.hpp"
#include "cli/commands/validate_command.hpp"
#include "cli/presenters/console_presenter.hpp"
#include "core/validation/truss_validator.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::cli::commands;
using namespace truss::core::validation;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

// ---------------------------------------------------------------------------
// Silent output sink
// ---------------------------------------------------------------------------

class VcSilentOutput : public truss::application::interfaces::IApplicationOutput {
public:
    void info(const std::string&) override {}
    void success(const std::string&) override {}
    void error(const std::string&) override {}
    void warn(const std::string&) override {}
};

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class ValidateCommandMockTest : public ::testing::Test {
protected:
    NiceMock<truss::test::MockTrussAnalysisFacade> mockFacade;
    VcSilentOutput silentOut;
    truss::cli::presenters::ConsolePresenter presenter{silentOut};

    std::string inputFile{"vc_mock_truss.json"};

    void SetUp() override {
        std::ofstream f(inputFile);
        f << "{}";
        f.close();
    }

    void TearDown() override { std::filesystem::remove(inputFile); }

    /** Helper: builds a ValidationResult that isValid() == true (no errors). */
    static ValidationResult makeValidResult() {
        ValidationResult r;
        r.addIssue(ValidationIssue{ValidationSeverity::Info, "General", "All checks passed"});
        return r;
    }

    /** Helper: builds an invalid ValidationResult containing an Error issue. */
    static ValidationResult makeInvalidResult(const std::string& category = "Unknown",
                                               const std::string& message = "Bad structure") {
        ValidationResult r;
        r.addIssue(ValidationIssue{ValidationSeverity::Error, category, message});
        return r;
    }
};

// ---------------------------------------------------------------------------
// Success path (isValid == true)
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationSuccess_ReturnsZero) {
    ON_CALL(mockFacade, validateFromFile(_)).WillByDefault(Return(makeValidResult()));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// Verbose success path – exercises the 8 category displayInfo lines
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationSuccess_VerboseMode_ReturnsZero) {
    ON_CALL(mockFacade, validateFromFile(_)).WillByDefault(Return(makeValidResult()));

    ValidateCommand cmd(mockFacade, presenter, inputFile, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 0);
}

// ---------------------------------------------------------------------------
// Failure path: all 4 severity labels in the switch
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_AllSeverities) {
    ValidationResult r;
    r.addIssue(ValidationIssue{ValidationSeverity::Fatal,   "Structural", "Fatal issue"});
    r.addIssue(ValidationIssue{ValidationSeverity::Error,   "Structural", "Error issue"});
    r.addIssue(ValidationIssue{ValidationSeverity::Warning, "Structural", "Warning issue"});
    r.addIssue(ValidationIssue{ValidationSeverity::Info,    "Structural", "Info issue"});

    ON_CALL(mockFacade, validateFromFile(_)).WillByDefault(Return(r));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    // Fatal + Error → isValid() = false → returns 1
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Suggestion blocks: hasSupport
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_SupportCategory_ShowsSupportSuggestions) {
    // category contains "support" → triggers hasSupport block
    ON_CALL(mockFacade, validateFromFile(_))
        .WillByDefault(Return(makeInvalidResult("support conditions", "Missing pin support")));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Suggestion blocks: hasMember
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_MemberCategory_ShowsMemberSuggestions) {
    ON_CALL(mockFacade, validateFromFile(_))
        .WillByDefault(Return(makeInvalidResult("member connectivity", "Zero-length member")));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Suggestion blocks: hasLoad
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_LoadCategory_ShowsLoadSuggestions) {
    ON_CALL(mockFacade, validateFromFile(_))
        .WillByDefault(Return(makeInvalidResult("load application", "No loads defined")));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Suggestion blocks: general (no support/member/load keyword in category)
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_NoCategory_ShowsGeneralSuggestions) {
    ON_CALL(mockFacade, validateFromFile(_))
        .WillByDefault(Return(makeInvalidResult("Numerical", "Ill-conditioned matrix")));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Multiple suggestion types in a single validation result
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_MultipleCategories) {
    ValidationResult r;
    r.addIssue(ValidationIssue{ValidationSeverity::Error, "support", "Insufficient supports"});
    r.addIssue(ValidationIssue{ValidationSeverity::Error, "member",  "Degenerate member"});
    r.addIssue(ValidationIssue{ValidationSeverity::Error, "load",    "No applied loads"});

    ON_CALL(mockFacade, validateFromFile(_)).WillByDefault(Return(r));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// Empty issues list but invalid (no issues but categorised as invalid via
// Fatal/Error count) – exercises the "issues.empty()" branch inside the
// failure block.
// Actually ValidationResult::isValid() checks Error+Fatal, so we need at least
// one such issue. To exercise hasFatal path: add a Fatal only.
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_ValidationFails_FatalOnly_ReturnsOne) {
    ValidationResult r;
    r.addIssue(ValidationIssue{ValidationSeverity::Fatal, "Structural", "Cannot analyse"});

    ON_CALL(mockFacade, validateFromFile(_)).WillByDefault(Return(r));

    ValidateCommand cmd(mockFacade, presenter, inputFile);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// validateInputFile() – directory branch (is_regular_file == false)
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_DirectoryAsInput_ReturnsOne) {
    namespace fs = std::filesystem;
    std::string dirPath = "vc_mock_tmpdir";
    fs::create_directory(dirPath);

    ValidateCommand cmd(mockFacade, presenter, dirPath);
    int rc = cmd.execute();

    fs::remove(dirPath);
    EXPECT_EQ(rc, 1);
}

// ---------------------------------------------------------------------------
// verbose + failure: verbose mode still returns 1 when validation fails
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_VerboseMode_ValidationFails_ReturnsOne) {
    ON_CALL(mockFacade, validateFromFile(_))
        .WillByDefault(Return(makeInvalidResult("support", "No supports")));

    ValidateCommand cmd(mockFacade, presenter, inputFile, /*verbose=*/true);
    EXPECT_EQ(cmd.execute(), 1);
}

// ---------------------------------------------------------------------------
// validateInputFile: non-existent file → returns 1
// ---------------------------------------------------------------------------

TEST_F(ValidateCommandMockTest, Execute_NonExistentFile_ReturnsOne) {
    ValidateCommand cmd(mockFacade, presenter, "/nonexistent_dir_xyz/truss.json");
    EXPECT_EQ(cmd.execute(), 1);
}
