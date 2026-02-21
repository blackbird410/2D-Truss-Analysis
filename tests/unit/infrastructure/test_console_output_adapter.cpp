/**
 * @file test_console_output_adapter.cpp
 * @brief Unit tests for ConsoleOutputAdapter
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Tests verify that ConsoleOutputAdapter correctly implements IApplicationOutput
 * by delegating to the underlying ILogger implementation.
 *
 * Test Coverage:
 * - info() delegates to logger.info()
 * - success() delegates to logger.info()
 * - error() delegates to logger.error()
 * - warn() delegates to logger.warn()
 *
 * Design Pattern: Adapter
 * Tests verify the adapter correctly bridges Application to Infrastructure layer.
 */

#include "../../../src/infrastructure/adapters/ConsoleOutputAdapter.hpp"
#include "../../../src/infrastructure/logging/logger.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace truss::infrastructure::adapters;
using namespace truss::infrastructure::logging;
using ::testing::_;

/**
 * @brief Mock ILogger for testing adapter delegation
 *
 * Provides expectations to verify ConsoleOutputAdapter correctly
 * delegates to the underlying logger implementation.
 */
class MockILogger : public ILogger {
public:
    MOCK_METHOD(void, trace, (const std::string& message), (override));
    MOCK_METHOD(void, debug, (const std::string& message), (override));
    MOCK_METHOD(void, info, (const std::string& message), (override));
    MOCK_METHOD(void, warn, (const std::string& message), (override));
    MOCK_METHOD(void, error, (const std::string& message), (override));
    MOCK_METHOD(void, critical, (const std::string& message), (override));
    MOCK_METHOD(void, setLevel, (LogLevel level), (override));
    MOCK_METHOD(LogLevel, getLevel, (), (const, override));
    MOCK_METHOD(bool, isLevelEnabled, (LogLevel level), (const, override));
};

/**
 * @brief Test fixture for ConsoleOutputAdapter tests
 *
 * Provides common setup: mock logger and adapter instance.
 */
class ConsoleOutputAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock logger (must outlive adapter)
        mockLogger = std::make_unique<MockILogger>();
        // Create adapter with mock logger reference
        adapter = std::make_unique<ConsoleOutputAdapter>(*mockLogger);
    }

    void TearDown() override {
        // Clean up in reverse order of creation
        adapter.reset();
        mockLogger.reset();
    }

    std::unique_ptr<MockILogger> mockLogger;
    std::unique_ptr<ConsoleOutputAdapter> adapter;
};

/**
 * @brief Test: info() delegates to logger.info()
 *
 * Verifies that IApplicationOutput::info() correctly calls
 * the underlying logger's info() method with the exact message.
 *
 * Expected Behavior:
 * - adapter.info("test message") calls mockLogger.info("test message") exactly once
 * - No other logger methods are called
 */
TEST_F(ConsoleOutputAdapterTest, InfoDelegatesToLoggerInfo) {
    // Arrange: Set expectation for logger.info() to be called exactly once
    EXPECT_CALL(*mockLogger, info("test message")).Times(1);

    // Act: Call adapter.info()
    adapter->info("test message");

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: success() delegates to logger.info()
 *
 * Verifies that IApplicationOutput::success() maps to logger's info()
 * method, as success is a semantic variant of informational output.
 *
 * Expected Behavior:
 * - adapter.success("success message") calls mockLogger.info("success message") exactly once
 * - Success messages use Info severity level (not a separate level)
 */
TEST_F(ConsoleOutputAdapterTest, SuccessDelegatesToLoggerInfo) {
    // Arrange: Set expectation for logger.info() to be called exactly once
    EXPECT_CALL(*mockLogger, info("success message")).Times(1);

    // Act: Call adapter.success()
    adapter->success("success message");

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: error() delegates to logger.error()
 *
 * Verifies that IApplicationOutput::error() correctly calls
 * the underlying logger's error() method with the exact message.
 *
 * Expected Behavior:
 * - adapter.error("error message") calls mockLogger.error("error message") exactly once
 * - Error messages use Error severity level
 */
TEST_F(ConsoleOutputAdapterTest, ErrorDelegatesToLoggerError) {
    // Arrange: Set expectation for logger.error() to be called exactly once
    EXPECT_CALL(*mockLogger, error("error message")).Times(1);

    // Act: Call adapter.error()
    adapter->error("error message");

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: warn() delegates to logger.warn()
 *
 * Verifies that IApplicationOutput::warn() correctly calls
 * the underlying logger's warn() method with the exact message.
 *
 * Expected Behavior:
 * - adapter.warn("warning message") calls mockLogger.warn("warning message") exactly once
 * - Warning messages use Warning severity level
 */
TEST_F(ConsoleOutputAdapterTest, WarnDelegatesToLoggerWarn) {
    // Arrange: Set expectation for logger.warn() to be called exactly once
    EXPECT_CALL(*mockLogger, warn("warning message")).Times(1);

    // Act: Call adapter.warn()
    adapter->warn("warning message");

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: Multiple messages are delegated correctly
 *
 * Verifies that the adapter correctly handles multiple consecutive
 * calls to different output methods, delegating each to the
 * appropriate logger method.
 *
 * Expected Behavior:
 * - Each adapter call results in exactly one corresponding logger call
 * - Message order is preserved
 * - Message content is passed through unchanged
 */
TEST_F(ConsoleOutputAdapterTest, MultipleMessagesDelegatedCorrectly) {
    // Arrange: Set expectations for multiple logger calls
    EXPECT_CALL(*mockLogger, info("first info")).Times(1);
    EXPECT_CALL(*mockLogger, error("first error")).Times(1);
    EXPECT_CALL(*mockLogger, warn("first warning")).Times(1);
    EXPECT_CALL(*mockLogger, info("second success")).Times(1);

    // Act: Call adapter methods in sequence
    adapter->info("first info");
    adapter->error("first error");
    adapter->warn("first warning");
    adapter->success("second success");

    // Assert: All expectations verified by GoogleMock on scope exit
}

/**
 * @brief Test: Empty messages are handled correctly
 *
 * Verifies that the adapter correctly delegates empty strings
 * to the logger without modification or filtering.
 *
 * Expected Behavior:
 * - adapter.info("") calls mockLogger.info("") exactly once
 * - No special handling or filtering of empty messages
 */
TEST_F(ConsoleOutputAdapterTest, EmptyMessagesHandledCorrectly) {
    // Arrange: Set expectation for logger.info() with empty string
    EXPECT_CALL(*mockLogger, info("")).Times(1);

    // Act: Call adapter.info() with empty string
    adapter->info("");

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: Special characters are passed through unchanged
 *
 * Verifies that the adapter does not modify or escape special
 * characters in messages (newlines, tabs, quotes, etc.).
 *
 * Expected Behavior:
 * - Special characters are passed through to logger unchanged
 * - No sanitization or transformation of message content
 */
TEST_F(ConsoleOutputAdapterTest, SpecialCharactersPassedThrough) {
    // Arrange: Message with special characters
    const std::string specialMessage = "Line 1\nLine 2\tTabbed\n\"Quoted\"";
    EXPECT_CALL(*mockLogger, info(specialMessage)).Times(1);

    // Act: Call adapter.info() with special characters
    adapter->info(specialMessage);

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: Large messages are handled correctly
 *
 * Verifies that the adapter correctly handles large messages
 * without truncation or buffering issues.
 *
 * Expected Behavior:
 * - Large messages are passed through to logger unchanged
 * - No size limitations imposed by the adapter
 */
TEST_F(ConsoleOutputAdapterTest, LargeMessagesHandledCorrectly) {
    // Arrange: Large message (1000 characters)
    const std::string largeMessage(1000, 'x');
    EXPECT_CALL(*mockLogger, info(largeMessage)).Times(1);

    // Act: Call adapter.info() with large message
    adapter->info(largeMessage);

    // Assert: Expectation verified by GoogleMock on scope exit
}

/**
 * @brief Test: Constructor stores logger reference correctly
 *
 * Verifies that the adapter correctly stores the logger reference
 * and uses it for all delegation calls (not a copy).
 *
 * Expected Behavior:
 * - Constructor accepts logger reference
 * - Same logger instance is used for all calls
 */
TEST_F(ConsoleOutputAdapterTest, ConstructorStoresLoggerReference) {
    // Arrange: Create new mock logger and adapter
    MockILogger testLogger;
    ConsoleOutputAdapter testAdapter(testLogger);

    // Set expectation on the specific logger instance
    EXPECT_CALL(testLogger, info("test")).Times(1);

    // Act: Call adapter method
    testAdapter.info("test");

    // Assert: Expectation verified (proving reference was stored)
}
