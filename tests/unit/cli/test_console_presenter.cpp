/**
 * @file test_console_presenter.cpp
 * @brief Unit tests for ConsolePresenter
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Tests verify that ConsolePresenter correctly formats output and delegates
 * to IApplicationOutput abstraction without adding redundant prefixes.
 *
 * Test Coverage:
 * - displayInfo() passes message without "INFO:" prefix
 * - displayError() passes message without "ERROR:" prefix
 * - displaySuccess() passes message without "SUCCESS:" prefix
 * - displayHeader() formats header correctly
 * - No direct I/O operations (all delegated)
 */

#include "../../../src/application/interfaces/IApplicationOutput.hpp"
#include "../../../src/cli/presenters/ConsolePresenter.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::cli::presenters;
using namespace truss::application::interfaces;
using ::testing::_;
using ::testing::AllOf;
using ::testing::AtLeast;
using ::testing::HasSubstr;
using ::testing::Not;

/**
 * @brief Mock IApplicationOutput for testing presenter delegation
 */
class MockApplicationOutput : public IApplicationOutput {
public:
    MOCK_METHOD(void, info, (const std::string& message), (override));
    MOCK_METHOD(void, success, (const std::string& message), (override));
    MOCK_METHOD(void, error, (const std::string& message), (override));
    MOCK_METHOD(void, warn, (const std::string& message), (override));
};

/**
 * @brief Test fixture for ConsolePresenter tests
 */
class ConsolePresenterTest : public ::testing::Test {
protected:
    MockApplicationOutput mockOutput;
};

/**
 * @test Verify displayInfo() passes message without "INFO:" prefix
 *
 * Rationale: Logger already adds [INFO] tag, adding "INFO:" creates
 * redundant output like "[INFO] INFO: message"
 */
TEST_F(ConsolePresenterTest, DisplayInfo_NoRedundantPrefix) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, info("Test message")).Times(1);

    presenter.displayInfo("Test message");
}

/**
 * @test Verify displayInfo() does NOT add "INFO:" prefix
 */
TEST_F(ConsolePresenterTest, DisplayInfo_MessagePassedVerbatim) {
    ConsolePresenter presenter(mockOutput);

    // Verify message does NOT contain "INFO:" prefix
    EXPECT_CALL(mockOutput, info(Not(HasSubstr("INFO:")))).Times(1);

    presenter.displayInfo("Usage:");
}

/**
 * @test Verify displayError() passes message without "ERROR:" prefix
 */
TEST_F(ConsolePresenterTest, DisplayError_NoRedundantPrefix) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, error("Invalid input")).Times(1);

    presenter.displayError("Invalid input");
}

/**
 * @test Verify displayError() does NOT add "ERROR:" prefix
 */
TEST_F(ConsolePresenterTest, DisplayError_MessagePassedVerbatim) {
    ConsolePresenter presenter(mockOutput);

    // Verify message does NOT contain "ERROR:" prefix
    EXPECT_CALL(mockOutput, error(Not(HasSubstr("ERROR:")))).Times(1);

    presenter.displayError("Command failed");
}

/**
 * @test Verify displaySuccess() passes message without "SUCCESS:" prefix
 */
TEST_F(ConsolePresenterTest, DisplaySuccess_NoRedundantPrefix) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, success("Analysis complete")).Times(1);

    presenter.displaySuccess("Analysis complete");
}

/**
 * @test Verify displaySuccess() does NOT add "SUCCESS:" prefix
 */
TEST_F(ConsolePresenterTest, DisplaySuccess_MessagePassedVerbatim) {
    ConsolePresenter presenter(mockOutput);

    // Verify message does NOT contain "SUCCESS:" prefix
    EXPECT_CALL(mockOutput, success(Not(HasSubstr("SUCCESS:")))).Times(1);

    presenter.displaySuccess("Operation succeeded");
}

/**
 * @test Verify displayHeader() formats header correctly
 */
TEST_F(ConsolePresenterTest, DisplayHeader_FormatsCorrectly) {
    ConsolePresenter presenter(mockOutput);

    // Verify each line is called separately with expected content
    // Note: The separator line appears twice (top and bottom)
    EXPECT_CALL(mockOutput, info("================================================="))
        .Times(2);  // Called twice - top and bottom separator
    EXPECT_CALL(mockOutput, info("       2D Truss Analysis Software v3.0.0       ")).Times(1);
    EXPECT_CALL(mockOutput, info("   Civil Engineering Software Solutions         ")).Times(1);
    EXPECT_CALL(mockOutput, info("")).Times(1);

    presenter.displayHeader();
}

/**
 * @test Verify displayHeader() uses multiple info() calls (one per line)
 */
TEST_F(ConsolePresenterTest, DisplayHeader_MultipleLinesCalls) {
    ConsolePresenter presenter(mockOutput);

    // Should make 5 separate calls to info() - one for each header line plus empty line
    EXPECT_CALL(mockOutput, info(_)).Times(5);

    presenter.displayHeader();
}

/**
 * @test Verify empty message handling
 */
TEST_F(ConsolePresenterTest, DisplayInfo_EmptyMessageHandled) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, info("")).Times(1);

    presenter.displayInfo("");
}

/**
 * @test Verify special characters passed correctly
 */
TEST_F(ConsolePresenterTest, DisplayInfo_SpecialCharactersPreserved) {
    ConsolePresenter presenter(mockOutput);

    const std::string specialMsg = "Test: 100% complete! (✓)";
    EXPECT_CALL(mockOutput, info(specialMsg)).Times(1);

    presenter.displayInfo(specialMsg);
}

/**
 * @test Verify multiple consecutive calls work correctly
 */
TEST_F(ConsolePresenterTest, MultipleDisplayInfoCalls_AllDelegated) {
    ConsolePresenter presenter(mockOutput);

    EXPECT_CALL(mockOutput, info("Line 1")).Times(1);
    EXPECT_CALL(mockOutput, info("Line 2")).Times(1);
    EXPECT_CALL(mockOutput, info("Line 3")).Times(1);

    presenter.displayInfo("Line 1");
    presenter.displayInfo("Line 2");
    presenter.displayInfo("Line 3");
}
