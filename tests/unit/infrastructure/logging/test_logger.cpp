/**
 * @file test_logger.cpp
 * @brief Unit tests for logging infrastructure
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "infrastructure/logging/logger.hpp"
#include "infrastructure/logging/console_logger.hpp"
#include "infrastructure/logging/file_logger.hpp"
#include "infrastructure/logging/logger_factory.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace truss::infrastructure::logging;

/**
 * @brief Test suite for ConsoleLogger
 */
class ConsoleLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any test files from previous runs
        cleanupTestFiles();
    }
    
    void TearDown() override {
        // Clean up test files after each test
        cleanupTestFiles();
    }
    
    void cleanupTestFiles() {
        const std::vector<std::string> testFiles = {
            "test_log.txt", "test_append.txt", "test_filter.txt",
            "test_shutdown.txt", "test_factory_file.txt", "test_default.txt"
        };
        for (const auto& file : testFiles) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }
};

TEST_F(ConsoleLoggerTest, LogLevelFiltering) {
    ConsoleLogger logger(LogLevel::Warning, false);
    
    // Should not log below warning level
    EXPECT_TRUE(logger.isLevelEnabled(LogLevel::Error));
    EXPECT_TRUE(logger.isLevelEnabled(LogLevel::Warning));
    EXPECT_FALSE(logger.isLevelEnabled(LogLevel::Info));
    EXPECT_FALSE(logger.isLevelEnabled(LogLevel::Debug));
    EXPECT_FALSE(logger.isLevelEnabled(LogLevel::Trace));
    
    // Change level
    logger.setLevel(LogLevel::Debug);
    EXPECT_EQ(logger.getLevel(), LogLevel::Debug);
    EXPECT_TRUE(logger.isLevelEnabled(LogLevel::Info));
    EXPECT_FALSE(logger.isLevelEnabled(LogLevel::Trace));
}

TEST_F(ConsoleLoggerTest, ColorCodes) {
    // Create logger with colors enabled
    ConsoleLogger colorLogger(LogLevel::Trace, true);
    EXPECT_EQ(colorLogger.getLevel(), LogLevel::Trace);
    
    // Create logger with colors disabled
    ConsoleLogger noColorLogger(LogLevel::Trace, false);
    EXPECT_EQ(noColorLogger.getLevel(), LogLevel::Trace);
    
    // Note: We can't easily test actual console output, but we can verify construction
}

TEST_F(ConsoleLoggerTest, AllLogLevels) {
    ConsoleLogger logger(LogLevel::Trace, false);
    
    // These should not throw
    EXPECT_NO_THROW(logger.trace("Trace message"));
    EXPECT_NO_THROW(logger.debug("Debug message"));
    EXPECT_NO_THROW(logger.info("Info message"));
    EXPECT_NO_THROW(logger.warn("Warning message"));
    EXPECT_NO_THROW(logger.error("Error message"));
    EXPECT_NO_THROW(logger.critical("Critical message"));
}

/**
 * @brief Test suite for FileLogger
 */
class FileLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        cleanupTestFiles();
    }
    
    void TearDown() override {
        cleanupTestFiles();
    }
    
    void cleanupTestFiles() {
        const std::vector<std::string> testFiles = {
            "test_log.txt", "test_append.txt", "test_filter.txt",
            "test_shutdown.txt", "test_factory_file.txt", "test_default.txt"
        };
        for (const auto& file : testFiles) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }
};

TEST_F(FileLoggerTest, FileCreation) {
    std::filesystem::path testFile = "test_log.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    {
        FileLogger logger(testFile, LogLevel::Info, false);
        logger.info("Test message");
    } // Logger destructor should close file
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Read file content
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_NE(content.find("Test message"), std::string::npos);
    EXPECT_NE(content.find("Logger initialized"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(FileLoggerTest, AppendMode) {
    std::filesystem::path testFile = "test_append.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    // First logger
    {
        FileLogger logger(testFile, LogLevel::Info, true);
        logger.info("First message");
    }
    
    // Second logger in append mode
    {
        FileLogger logger(testFile, LogLevel::Info, true);
        logger.info("Second message");
    }
    
    // Read file content
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_NE(content.find("First message"), std::string::npos);
    EXPECT_NE(content.find("Second message"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(FileLoggerTest, LogLevelFiltering) {
    std::filesystem::path testFile = "test_filter.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    {
        FileLogger logger(testFile, LogLevel::Warning, false);
        
        logger.trace("Trace message");    // Should not be logged
        logger.debug("Debug message");    // Should not be logged
        logger.info("Info message");      // Should not be logged
        logger.warn("Warning message");   // Should be logged
        logger.error("Error message");    // Should be logged
    }
    
    // Read file content
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_EQ(content.find("Trace message"), std::string::npos);
    EXPECT_EQ(content.find("Debug message"), std::string::npos);
    EXPECT_EQ(content.find("Info message"), std::string::npos);
    EXPECT_NE(content.find("Warning message"), std::string::npos);
    EXPECT_NE(content.find("Error message"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(FileLoggerTest, ShutdownMessage) {
    std::filesystem::path testFile = "test_shutdown.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    {
        FileLogger logger(testFile, LogLevel::Trace, false);
        logger.info("Test message");
    } // Destructor should log shutdown message
    
    // Read file content
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_NE(content.find("Logger shutting down"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}

/**
 * @brief Test suite for LoggerFactory
 */
class LoggerFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        cleanupTestFiles();
    }
    
    void TearDown() override {
        cleanupTestFiles();
    }
    
    void cleanupTestFiles() {
        const std::vector<std::string> testFiles = {
            "test_log.txt", "test_append.txt", "test_filter.txt",
            "test_shutdown.txt", "test_factory_file.txt", "test_default.txt"
        };
        for (const auto& file : testFiles) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }
};

TEST_F(LoggerFactoryTest, CreateConsoleLogger) {
    auto logger = LoggerFactory::createConsoleLogger(LogLevel::Info, false);
    
    ASSERT_NE(logger, nullptr);
    EXPECT_EQ(logger->getLevel(), LogLevel::Info);
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Info));
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Debug));
}

TEST_F(LoggerFactoryTest, CreateFileLogger) {
    std::filesystem::path testFile = "test_factory_file.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    {
        auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Debug, false);
        
        ASSERT_NE(logger, nullptr);
        EXPECT_EQ(logger->getLevel(), LogLevel::Debug);
        
        logger->info("Factory test message");
    }
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(LoggerFactoryTest, CreateDefaultLogger) {
    std::filesystem::path testFile = "test_default.txt";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
    
    {
        auto logger = LoggerFactory::createDefaultLogger(testFile, LogLevel::Warning, false);
        
        ASSERT_NE(logger, nullptr);
        // createDefaultLogger MUST respect the requested log level
        EXPECT_EQ(logger->getLevel(), LogLevel::Warning);
        
        logger->warn("Default logger test");
    }
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Read file content
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    EXPECT_NE(content.find("Default logger test"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(LoggerFactoryTest, CreateNullLogger) {
    auto logger = LoggerFactory::createNullLogger();
    
    ASSERT_NE(logger, nullptr);
    // NullLogger contract: Trace level (most permissive) - accepts all messages, discards all
    EXPECT_EQ(logger->getLevel(), LogLevel::Trace);
    
    // These should not throw or do anything
    EXPECT_NO_THROW(logger->trace("Should be discarded"));
    EXPECT_NO_THROW(logger->debug("Should be discarded"));
    EXPECT_NO_THROW(logger->info("Should be discarded"));
    EXPECT_NO_THROW(logger->warn("Should be discarded"));
    EXPECT_NO_THROW(logger->error("Should be discarded"));
    EXPECT_NO_THROW(logger->critical("Should be discarded"));
}

TEST_F(LoggerFactoryTest, DefaultLoggerFallback) {
    // Try to create logger with invalid path (should fallback to console only)
    std::filesystem::path invalidPath = "/nonexistent/directory/test.log";
    
    auto logger = LoggerFactory::createDefaultLogger(invalidPath, LogLevel::Info, false);
    
    ASSERT_NE(logger, nullptr);
    
    // Logger should still work (console fallback)
    EXPECT_NO_THROW(logger->info("This should work via console fallback"));
}

/**
 * @brief Test suite for LogLevel enum
 */
class LogLevelTest : public ::testing::Test {
};

TEST_F(LogLevelTest, LogLevelOrdering) {
    EXPECT_LT(LogLevel::Trace, LogLevel::Debug);
    EXPECT_LT(LogLevel::Debug, LogLevel::Info);
    EXPECT_LT(LogLevel::Info, LogLevel::Warning);
    EXPECT_LT(LogLevel::Warning, LogLevel::Error);
    EXPECT_LT(LogLevel::Error, LogLevel::Critical);
}
