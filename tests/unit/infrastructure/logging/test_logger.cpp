/**
 * @file test_logger.cpp
 * @brief Unit tests for logging infrastructure
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "../../../../src/infrastructure/logging/logger.hpp"
#include "../../../../src/infrastructure/logging/console_logger.hpp"
#include "../../../../src/infrastructure/logging/file_logger.hpp"
#include "../../../../src/infrastructure/logging/logger_factory.hpp"
#include "../../../TestFramework.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace infrastructure::logging;

/**
 * @brief Test suite for ConsoleLogger
 */
class ConsoleLoggerTests {
public:
    static void testLogLevelFiltering() {
        TEST_SECTION("ConsoleLogger - Log Level Filtering");
        
        ConsoleLogger logger(LogLevel::Warning, false);
        
        // Should not log below warning level
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Error) == true, "Error should be enabled");
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Warning) == true, "Warning should be enabled");
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Info) == false, "Info should be disabled");
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Debug) == false, "Debug should be disabled");
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Trace) == false, "Trace should be disabled");
        
        // Change level
        logger.setLevel(LogLevel::Debug);
        TEST_ASSERT(logger.getLevel() == LogLevel::Debug, "Level should be Debug");
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Info) == true, "Info should now be enabled");
        TEST_ASSERT(logger.isLevelEnabled(LogLevel::Trace) == false, "Trace should still be disabled");
        
        TEST_PASS("ConsoleLogger log level filtering works correctly");
    }
    
    static void testColorCodes() {
        TEST_SECTION("ConsoleLogger - Color Codes");
        
        // Create logger with colors enabled
        ConsoleLogger colorLogger(LogLevel::Trace, true);
        TEST_ASSERT(colorLogger.getLevel() == LogLevel::Trace, "Level should be Trace");
        
        // Create logger with colors disabled
        ConsoleLogger noColorLogger(LogLevel::Trace, false);
        TEST_ASSERT(noColorLogger.getLevel() == LogLevel::Trace, "Level should be Trace");
        
        // Note: We can't easily test actual console output, but we can verify construction
        TEST_PASS("ConsoleLogger color configuration works correctly");
    }
    
    static void testAllLogLevels() {
        TEST_SECTION("ConsoleLogger - All Log Levels");
        
        ConsoleLogger logger(LogLevel::Trace, false);
        
        // These should not throw
        logger.trace("Trace message");
        logger.debug("Debug message");
        logger.info("Info message");
        logger.warn("Warning message");
        logger.error("Error message");
        logger.critical("Critical message");
        
        TEST_PASS("ConsoleLogger handles all log levels without throwing");
    }
};

/**
 * @brief Test suite for FileLogger
 */
class FileLoggerTests {
public:
    static void testFileCreation() {
        TEST_SECTION("FileLogger - File Creation");
        
        std::filesystem::path testFile = "test_log.txt";
        
        // Clean up any existing test file
        if (std::filesystem::exists(testFile)) {
            std::filesystem::remove(testFile);
        }
        
        {
            FileLogger logger(testFile, LogLevel::Info, false);
            logger.info("Test message");
        } // Logger destructor should close file
        
        TEST_ASSERT(std::filesystem::exists(testFile), "Log file should be created");
        
        // Read file content
        std::ifstream file(testFile);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        TEST_ASSERT(content.find("Test message") != std::string::npos, 
                   "Log file should contain test message");
        TEST_ASSERT(content.find("Logger initialized") != std::string::npos,
                   "Log file should contain initialization message");
        
        // Clean up
        std::filesystem::remove(testFile);
        
        TEST_PASS("FileLogger creates and writes to file correctly");
    }
    
    static void testAppendMode() {
        TEST_SECTION("FileLogger - Append Mode");
        
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
        
        TEST_ASSERT(content.find("First message") != std::string::npos,
                   "Log file should contain first message");
        TEST_ASSERT(content.find("Second message") != std::string::npos,
                   "Log file should contain second message");
        
        // Clean up
        std::filesystem::remove(testFile);
        
        TEST_PASS("FileLogger append mode works correctly");
    }
    
    static void testLogLevelFiltering() {
        TEST_SECTION("FileLogger - Log Level Filtering");
        
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
        
        TEST_ASSERT(content.find("Trace message") == std::string::npos,
                   "Trace message should not be logged");
        TEST_ASSERT(content.find("Debug message") == std::string::npos,
                   "Debug message should not be logged");
        TEST_ASSERT(content.find("Info message") == std::string::npos,
                   "Info message should not be logged");
        TEST_ASSERT(content.find("Warning message") != std::string::npos,
                   "Warning message should be logged");
        TEST_ASSERT(content.find("Error message") != std::string::npos,
                   "Error message should be logged");
        
        // Clean up
        std::filesystem::remove(testFile);
        
        TEST_PASS("FileLogger log level filtering works correctly");
    }
    
    static void testShutdownMessage() {
        TEST_SECTION("FileLogger - Shutdown Message");
        
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
        
        TEST_ASSERT(content.find("Logger shutting down") != std::string::npos,
                   "Log file should contain shutdown message");
        
        // Clean up
        std::filesystem::remove(testFile);
        
        TEST_PASS("FileLogger logs shutdown message correctly");
    }
};

/**
 * @brief Test suite for LoggerFactory
 */
class LoggerFactoryTests {
public:
    static void testCreateConsoleLogger() {
        TEST_SECTION("LoggerFactory - Create Console Logger");
        
        auto logger = LoggerFactory::createConsoleLogger(LogLevel::Info, false);
        
        TEST_ASSERT(logger != nullptr, "Console logger should not be null");
        TEST_ASSERT(logger->getLevel() == LogLevel::Info, "Logger level should be Info");
        TEST_ASSERT(logger->isLevelEnabled(LogLevel::Info) == true, "Info should be enabled");
        TEST_ASSERT(logger->isLevelEnabled(LogLevel::Debug) == false, "Debug should be disabled");
        
        TEST_PASS("LoggerFactory creates console logger correctly");
    }
    
    static void testCreateFileLogger() {
        TEST_SECTION("LoggerFactory - Create File Logger");
        
        std::filesystem::path testFile = "test_factory_file.txt";
        
        // Clean up any existing test file
        if (std::filesystem::exists(testFile)) {
            std::filesystem::remove(testFile);
        }
        
        {
            auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Debug, false);
            
            TEST_ASSERT(logger != nullptr, "File logger should not be null");
            TEST_ASSERT(logger->getLevel() == LogLevel::Debug, "Logger level should be Debug");
            
            logger->info("Factory test message");
        }
        
        TEST_ASSERT(std::filesystem::exists(testFile), "Log file should be created");
        
        // Clean up
        std::filesystem::remove(testFile);
        
        TEST_PASS("LoggerFactory creates file logger correctly");
    }
    
    static void testCreateDefaultLogger() {
        TEST_SECTION("LoggerFactory - Create Default Logger");
        
        std::filesystem::path testFile = "test_default.txt";
        
        // Clean up any existing test file
        if (std::filesystem::exists(testFile)) {
            std::filesystem::remove(testFile);
        }
        
        {
            auto logger = LoggerFactory::createDefaultLogger(testFile, LogLevel::Warning, false);
            
            TEST_ASSERT(logger != nullptr, "Default logger should not be null");
            TEST_ASSERT(logger->getLevel() == LogLevel::Warning, "Logger level should be Warning");
            
            logger->warn("Default logger test");
        }
        
        TEST_ASSERT(std::filesystem::exists(testFile), "Log file should be created");
        
        // Read file content
        std::ifstream file(testFile);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        TEST_ASSERT(content.find("Default logger test") != std::string::npos,
                   "Log file should contain test message");
        
        // Clean up
        std::filesystem::remove(testFile);
        
        TEST_PASS("LoggerFactory creates default (composite) logger correctly");
    }
    
    static void testCreateNullLogger() {
        TEST_SECTION("LoggerFactory - Create Null Logger");
        
        auto logger = LoggerFactory::createNullLogger();
        
        TEST_ASSERT(logger != nullptr, "Null logger should not be null");
        TEST_ASSERT(logger->getLevel() == LogLevel::Trace, "Null logger should accept all levels");
        
        // These should not throw or do anything
        logger->trace("Should be discarded");
        logger->debug("Should be discarded");
        logger->info("Should be discarded");
        logger->warn("Should be discarded");
        logger->error("Should be discarded");
        logger->critical("Should be discarded");
        
        TEST_PASS("LoggerFactory creates null logger correctly");
    }
    
    static void testDefaultLoggerFallback() {
        TEST_SECTION("LoggerFactory - Default Logger File Creation Failure");
        
        // Try to create logger with invalid path (should fallback to console only)
        std::filesystem::path invalidPath = "/nonexistent/directory/test.log";
        
        auto logger = LoggerFactory::createDefaultLogger(invalidPath, LogLevel::Info, false);
        
        TEST_ASSERT(logger != nullptr, "Logger should not be null even with invalid path");
        
        // Logger should still work (console fallback)
        logger->info("This should work via console fallback");
        
        TEST_PASS("LoggerFactory handles file creation failure gracefully");
    }
};

/**
 * @brief Test suite for LogLevel enum
 */
class LogLevelTests {
public:
    static void testLogLevelOrdering() {
        TEST_SECTION("LogLevel - Ordering");
        
        TEST_ASSERT(LogLevel::Trace < LogLevel::Debug, "Trace < Debug");
        TEST_ASSERT(LogLevel::Debug < LogLevel::Info, "Debug < Info");
        TEST_ASSERT(LogLevel::Info < LogLevel::Warning, "Info < Warning");
        TEST_ASSERT(LogLevel::Warning < LogLevel::Error, "Warning < Error");
        TEST_ASSERT(LogLevel::Error < LogLevel::Critical, "Error < Critical");
        
        TEST_PASS("LogLevel enum ordering is correct");
    }
};

int main() {
    TEST_HEADER("Infrastructure Logging System Tests");
    
    // ConsoleLogger tests
    ConsoleLoggerTests::testLogLevelFiltering();
    ConsoleLoggerTests::testColorCodes();
    ConsoleLoggerTests::testAllLogLevels();
    
    // FileLogger tests
    FileLoggerTests::testFileCreation();
    FileLoggerTests::testAppendMode();
    FileLoggerTests::testLogLevelFiltering();
    FileLoggerTests::testShutdownMessage();
    
    // LoggerFactory tests
    LoggerFactoryTests::testCreateConsoleLogger();
    LoggerFactoryTests::testCreateFileLogger();
    LoggerFactoryTests::testCreateDefaultLogger();
    LoggerFactoryTests::testCreateNullLogger();
    LoggerFactoryTests::testDefaultLoggerFallback();
    
    // LogLevel tests
    LogLevelTests::testLogLevelOrdering();
    
    TEST_SUMMARY();
    
    return TEST_GET_RETURN_CODE();
}
