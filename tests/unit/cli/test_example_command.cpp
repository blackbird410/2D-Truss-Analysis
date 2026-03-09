/**
 * @file test_example_command.cpp
 * @brief Unit tests for ExampleCommand.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Test Coverage Targets:
 * - getName() / getDescription()
 * - execute() → success path (non-verbose and verbose)
 * - execute() → failure path when analysis fails  (m_verbose false)
 * - execute() → failure path when analysis fails  (m_verbose true – unreachable
 *               after early return, but the !analysisResult branch is the key)
 *
 * Design:
 * - Use real TrussAnalysisFacade for success-path tests (integration in
 *   unit-test clothing – acceptable here because ExampleCommand builds a
 *   hardcoded, always-valid truss).
 * - Use MockTrussAnalysisFacade + NiceMock to drive the failure branch without
 *   touching the filesystem or a real solver.
 */

#include "../../../src/application/interfaces/iapplication_output.hpp"
#include "../../../src/cli/commands/example_command.hpp"
#include "../../../src/cli/presenters/console_presenter.hpp"
#include "../../../src/interface/truss_analysis_facade.hpp"
#include "mocks/mock_truss_analysis_facade.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::cli::commands;
using namespace truss::interface;
using ::testing::NiceMock;
using ::testing::Return;

// ─────────────────────────────────────────────────────────────────────────────
// Minimal IApplicationOutput sink (discards all output)
// ─────────────────────────────────────────────────────────────────────────────

class SilentOutput : public truss::application::interfaces::IApplicationOutput {
public:
    void info(const std::string&) override {}
    void success(const std::string&) override {}
    void error(const std::string&) override {}
    void warn(const std::string&) override {}
};

// ─────────────────────────────────────────────────────────────────────────────
// Test fixture
// ─────────────────────────────────────────────────────────────────────────────

class ExampleCommandTest : public ::testing::Test {
protected:
    SilentOutput output;
    truss::cli::presenters::ConsolePresenter presenter{output};
    TrussAnalysisFacade realFacade;
};

// ─────────────────────────────────────────────────────────────────────────────
// Metadata tests
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ExampleCommandTest, GetName_ReturnsExample) {
    ExampleCommand cmd(realFacade, presenter);
    EXPECT_EQ(cmd.getName(), "example");
}

TEST_F(ExampleCommandTest, GetDescription_IsNonEmpty) {
    ExampleCommand cmd(realFacade, presenter);
    std::string desc = cmd.getDescription();
    EXPECT_FALSE(desc.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// Success path – non-verbose (uses real facade + real solver)
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ExampleCommandTest, Execute_NonVerbose_SuccessReturnsZero) {
    ExampleCommand cmd(realFacade, presenter, /*verbose=*/false);
    int exitCode = cmd.execute();
    EXPECT_EQ(exitCode, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Success path – verbose mode exercises both m_verbose branches
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ExampleCommandTest, Execute_VerboseMode_SuccessReturnsZero) {
    ExampleCommand cmd(realFacade, presenter, /*verbose=*/true);
    int exitCode = cmd.execute();
    // verbose=true hits all three verbose display branches in execute()
    EXPECT_EQ(exitCode, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Failure path – analysis fails (mock returns Failure result)
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(ExampleCommandTest, Execute_AnalysisFails_ReturnsOne) {
    NiceMock<truss::test::MockTrussAnalysisFacade> mockFacade;

    EXPECT_CALL(mockFacade, analyzeInteractive(::testing::_, ::testing::_))
        .WillOnce(Return(AnalysisWorkflowResult::Failure("Solver exploded")));

    ExampleCommand cmd(mockFacade, presenter, /*verbose=*/false);
    int exitCode = cmd.execute();
    EXPECT_EQ(exitCode, 1);
}

TEST_F(ExampleCommandTest, Execute_AnalysisFailsVerbose_ReturnsOne) {
    // verbose=true: the "Truss structure created" display fires, but the second
    // verbose branch ("Analysis completed successfully.") is NOT reached because
    // we return early. Just verifying the exit code.
    NiceMock<truss::test::MockTrussAnalysisFacade> mockFacade;

    EXPECT_CALL(mockFacade, analyzeInteractive(::testing::_, ::testing::_))
        .WillOnce(Return(AnalysisWorkflowResult::Failure("No solver")));

    ExampleCommand cmd(mockFacade, presenter, /*verbose=*/true);
    int exitCode = cmd.execute();
    EXPECT_EQ(exitCode, 1);
}
