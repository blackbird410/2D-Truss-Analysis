/**
 * @file test_help_command.cpp
 * @brief Unit tests for HelpCommand
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Tests verify that HelpCommand correctly uses injected ConsolePresenter
 * to display help information without direct I/O operations.
 *
 * Test Coverage:
 * - execute() returns success code (0)
 * - execute() delegates to presenter (no direct std::cout)
 * - Usage information displayed via presenter
 * - Command list displayed via presenter
 * - Examples displayed via presenter
 * - Empty command list handled gracefully
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../src/cli/commands/HelpCommand.hpp"
#include "../../../src/cli/presenters/ConsolePresenter.hpp"
#include "../../../src/application/interfaces/IApplicationOutput.hpp"

using namespace truss::cli::commands;
using namespace truss::cli::presenters;
using namespace truss::application::interfaces;
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::AtLeast;
using ::testing::Return;

/**
 * @brief Mock IApplicationOutput for testing
 */
class MockApplicationOutput : public IApplicationOutput {
public:
    MOCK_METHOD(void, info, (const std::string& message), (override));
    MOCK_METHOD(void, success, (const std::string& message), (override));
    MOCK_METHOD(void, error, (const std::string& message), (override));
    MOCK_METHOD(void, warn, (const std::string& message), (override));
};

/**
 * @brief Mock ICommand for testing command list display
 */
class MockCommand : public ICommand {
public:
    MOCK_METHOD(int, execute, (), (override));
    MOCK_METHOD(std::string, getName, (), (const, override));
    MOCK_METHOD(std::string, getDescription, (), (const, override));
};

/**
 * @brief Test fixture for HelpCommand tests
 */
class HelpCommandTest : public ::testing::Test {
protected:
    MockApplicationOutput mockOutput;
    
    void SetUp() override {
        // Common setup if needed
    }
};

/**
 * @test Verify execute() returns success code (0)
 */
TEST_F(HelpCommandTest, Execute_ReturnsSuccessCode) {
    ConsolePresenter presenter(mockOutput);
    std::vector<ICommand*> commands;
    HelpCommand helpCmd(presenter, commands);
    
    // Allow any number of info() calls (we're testing return code)
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    
    int exitCode = helpCmd.execute();
    
    EXPECT_EQ(exitCode, 0);
}

/**
 * @test Verify usage information displayed via presenter
 */
TEST_F(HelpCommandTest, Execute_DisplaysUsageViaPresenter) {
    ConsolePresenter presenter(mockOutput);
    std::vector<ICommand*> commands;
    HelpCommand helpCmd(presenter, commands);
    
    // Allow any calls, but verify "Usage:" is among them
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("Usage")))
        .Times(AtLeast(1));
    
    helpCmd.execute();
}

/**
 * @test Verify options information displayed
 */
TEST_F(HelpCommandTest, Execute_DisplaysOptions) {
    ConsolePresenter presenter(mockOutput);
    std::vector<ICommand*> commands;
    HelpCommand helpCmd(presenter, commands);
    
    // Allow any calls
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    // Verify specific content displayed
    EXPECT_CALL(mockOutput, info(HasSubstr("Options")))
        .Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("--help")))
        .Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("--verbose")))
        .Times(AtLeast(1));
    
    helpCmd.execute();
}

/**
 * @test Verify command list displayed when commands provided
 */
TEST_F(HelpCommandTest, Execute_DisplaysCommandList) {
    ConsolePresenter presenter(mockOutput);
    
    MockCommand mockCmd1;
    EXPECT_CALL(mockCmd1, getName())
        .WillRepeatedly(Return("example"));
    EXPECT_CALL(mockCmd1, getDescription())
        .WillRepeatedly(Return("Run example analysis"));
    
    std::vector<ICommand*> commands = { &mockCmd1 };
    HelpCommand helpCmd(presenter, commands);
    
    // Allow any calls
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    // Verify "Available Commands:" header displayed
    EXPECT_CALL(mockOutput, info(HasSubstr("Available Commands")))
        .Times(AtLeast(1));
    // Verify command name displayed
    EXPECT_CALL(mockOutput, info(HasSubstr("example")))
        .Times(AtLeast(1));
    
    helpCmd.execute();
}

/**
 * @test Verify examples section displayed
 */
TEST_F(HelpCommandTest, Execute_DisplaysExamples) {
    ConsolePresenter presenter(mockOutput);
    std::vector<ICommand*> commands;
    HelpCommand helpCmd(presenter, commands);
    
    // Allow any calls
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    // Verify examples displayed
    EXPECT_CALL(mockOutput, info(HasSubstr("Examples")))
        .Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("TrussAnalysisCLI example")))
        .Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("TrussAnalysisCLI help")))
        .Times(AtLeast(1));
    
    helpCmd.execute();
}

/**
 * @test Verify empty command list handled gracefully
 */
TEST_F(HelpCommandTest, Execute_HandlesEmptyCommandList) {
    ConsolePresenter presenter(mockOutput);
    std::vector<ICommand*> commands; // Empty list
    HelpCommand helpCmd(presenter, commands);
    
    // Should still display usage and examples, just no commands
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    
    int exitCode = helpCmd.execute();
    
    EXPECT_EQ(exitCode, 0);
}

/**
 * @test Verify null commands in list skipped safely
 */
TEST_F(HelpCommandTest, Execute_SkipsNullCommands) {
    ConsolePresenter presenter(mockOutput);
    
    MockCommand mockCmd1;
    EXPECT_CALL(mockCmd1, getName())
        .WillRepeatedly(Return("test"));
    EXPECT_CALL(mockCmd1, getDescription())
        .WillRepeatedly(Return("Test command"));
    
    std::vector<ICommand*> commands = { &mockCmd1, nullptr, nullptr };
    HelpCommand helpCmd(presenter, commands);
    
    // Should handle nullptrs without crashing
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    
    int exitCode = helpCmd.execute();
    
    EXPECT_EQ(exitCode, 0);
}

/**
 * @test Verify multiple commands displayed correctly
 */
TEST_F(HelpCommandTest, Execute_DisplaysMultipleCommands) {
    ConsolePresenter presenter(mockOutput);
    
    MockCommand mockCmd1, mockCmd2, mockCmd3;
    
    EXPECT_CALL(mockCmd1, getName()).WillRepeatedly(Return("example"));
    EXPECT_CALL(mockCmd1, getDescription()).WillRepeatedly(Return("Run example"));
    
    EXPECT_CALL(mockCmd2, getName()).WillRepeatedly(Return("help"));
    EXPECT_CALL(mockCmd2, getDescription()).WillRepeatedly(Return("Show help"));
    
    EXPECT_CALL(mockCmd3, getName()).WillRepeatedly(Return("analyze"));
    EXPECT_CALL(mockCmd3, getDescription()).WillRepeatedly(Return("Analyze truss"));
    
    std::vector<ICommand*> commands = { &mockCmd1, &mockCmd2, &mockCmd3 };
    HelpCommand helpCmd(presenter, commands);
    
    // Allow any calls
    EXPECT_CALL(mockOutput, info(_))
        .Times(AtLeast(1));
    // Verify all command names displayed
    EXPECT_CALL(mockOutput, info(HasSubstr("example"))).Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("help"))).Times(AtLeast(1));
    EXPECT_CALL(mockOutput, info(HasSubstr("analyze"))).Times(AtLeast(1));
    
    helpCmd.execute();
}

/**
 * @test Verify HelpCommand name and description correct
 */
TEST_F(HelpCommandTest, Metadata_CorrectNameAndDescription) {
    ConsolePresenter presenter(mockOutput);
    std::vector<ICommand*> commands;
    HelpCommand helpCmd(presenter, commands);
    
    EXPECT_EQ(helpCmd.getName(), "help");
    EXPECT_EQ(helpCmd.getDescription(), "Display this help information");
}
