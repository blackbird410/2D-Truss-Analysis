/**
 * @file test_logger_advanced.cpp
 * @brief Advanced tests for logging infrastructure - edge cases and complex scenarios
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-21
 */

#include "infrastructure/logging/logger.hpp"
#include "infrastructure/logging/console_logger.hpp"
#include "infrastructure/logging/file_logger.hpp"
#include "infrastructure/logging/logger_factory.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <atomic>

using namespace truss::infrastructure::logging;

class LoggerAdvancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        cleanupTestFiles();
    }
    
    void TearDown() override {
        cleanupTestFiles();
    }
    
    void cleanupTestFiles() {
        const std::vector<std::string> testFiles = {
            "test_concurrent.txt", "test_very_long.txt", "test_special_chars.txt",
            "test_unicode.txt", "test_rapid_fire.txt", "test_level_change.txt",
            "test_empty_msg.txt", "test_multiline.txt", "test_large_file.txt",
            "test_readonly.txt", "test_nested_dir/test.txt"
        };
        
        for (const auto& file : testFiles) {
            if (std::filesystem::exists(file)) {
                std::filesystem::permissions(file, 
                    std::filesystem::perms::owner_write,
                    std::filesystem::perm_options::add);
                std::filesystem::remove(file);
            }
        }
        
        // Clean up test directory
        if (std::filesystem::exists("test_nested_dir")) {
            std::filesystem::remove_all("test_nested_dir");
        }
    }
    
    std::string readFileContent(const std::filesystem::path& path) {
        std::ifstream file(path);
        return std::string((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    }
    
    size_t countLinesInFile(const std::filesystem::path& path) {
        std::ifstream file(path);
        size_t count = 0;
        std::string line;
        while (std::getline(file, line)) {
            ++count;
        }
        return count;
    }
};

// ============================================================================
// Test 8.4.1: Concurrent Logging Tests
// ============================================================================

TEST_F(LoggerAdvancedTest, ConcurrentFileLogging) {
    std::filesystem::path testFile = "test_concurrent.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    const int numThreads = 10;
    const int messagesPerThread = 100;
    std::vector<std::thread> threads;
    std::atomic<int> completedThreads{0};
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i, &completedThreads]() {
            for (int j = 0; j < messagesPerThread; ++j) {
                logger->info("Thread " + std::to_string(i) + " message " + std::to_string(j));
            }
            completedThreads++;
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(completedThreads, numThreads);
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Count log lines (excluding initialization/shutdown messages)
    auto content = readFileContent(testFile);
    size_t messageCount = 0;
    for (int i = 0; i < numThreads; ++i) {
        for (int j = 0; j < messagesPerThread; ++j) {
            std::string expected = "Thread " + std::to_string(i) + " message " + std::to_string(j);
            if (content.find(expected) != std::string::npos) {
                messageCount++;
            }
        }
    }
    
    EXPECT_EQ(messageCount, numThreads * messagesPerThread);
}

TEST_F(LoggerAdvancedTest, ConcurrentLevelChanges) {
    auto logger = LoggerFactory::createConsoleLogger(LogLevel::Info, false);
    
    std::vector<std::thread> threads;
    const int numThreads = 5;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i]() {
            for (int j = 0; j < 50; ++j) {
                // Alternate between changing level and logging
                if (j % 2 == 0) {
                    logger->setLevel(static_cast<LogLevel>(i % 6));
                } else {
                    logger->info("Thread " + std::to_string(i) + " message");
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should not crash
    EXPECT_TRUE(true);
}

// ============================================================================
// Test 8.4.2: Message Content Edge Cases
// ============================================================================

TEST_F(LoggerAdvancedTest, VeryLongMessage) {
    std::filesystem::path testFile = "test_very_long.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    // Create a 10KB message
    std::string longMessage(10000, 'A');
    longMessage += " END";
    
    EXPECT_NO_THROW(logger->info(longMessage));
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("END"), std::string::npos);
}

TEST_F(LoggerAdvancedTest, SpecialCharactersInMessage) {
    std::filesystem::path testFile = "test_special_chars.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    // Test various special characters
    logger->info("Tab:\t newline:\n quote:\" backslash:\\ null:");
    logger->info("Symbols: !@#$%^&*()_+-=[]{}|;:',.<>?/~`");
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Symbols:"), std::string::npos);
}

TEST_F(LoggerAdvancedTest, UnicodeCharacters) {
    std::filesystem::path testFile = "test_unicode.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    logger->info("Unicode: 中文 日本語 한글 Ελληνικά");
    logger->info("Math symbols: ∑ ∫ √ ∞ ≈ ≠");
    logger->info("Emoji: 😀 🚀 ⚠️");
    
    auto content = readFileContent(testFile);
    // Just verify file exists and has content
    EXPECT_GT(content.size(), 0);
}

TEST_F(LoggerAdvancedTest, EmptyMessage) {
    std::filesystem::path testFile = "test_empty_msg.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    EXPECT_NO_THROW(logger->info(""));
    EXPECT_NO_THROW(logger->warn(""));
    EXPECT_NO_THROW(logger->error(""));
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
}

TEST_F(LoggerAdvancedTest, MultilineMessage) {
    std::filesystem::path testFile = "test_multiline.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    std::string multiline = "Line 1\nLine 2\nLine 3\n";
    logger->info(multiline);
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Line 1"), std::string::npos);
    EXPECT_NE(content.find("Line 2"), std::string::npos);
    EXPECT_NE(content.find("Line 3"), std::string::npos);
}

// ============================================================================
// Test 8.4.3: Rapid Logging and Performance
// ============================================================================

TEST_F(LoggerAdvancedTest, RapidFireLogging) {
    std::filesystem::path testFile = "test_rapid_fire.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    const int numMessages = 1000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numMessages; ++i) {
        logger->info("Message " + std::to_string(i));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time (< 5 seconds)
    EXPECT_LT(duration.count(), 5000);
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Message 0"), std::string::npos);
    EXPECT_NE(content.find("Message 999"), std::string::npos);
}

TEST_F(LoggerAdvancedTest, MixedLevelRapidLogging) {
    std::filesystem::path testFile = "test_level_change.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Trace, false);
    
    for (int i = 0; i < 100; ++i) {
        switch (i % 6) {
            case 0: logger->trace("Trace " + std::to_string(i)); break;
            case 1: logger->debug("Debug " + std::to_string(i)); break;
            case 2: logger->info("Info " + std::to_string(i)); break;
            case 3: logger->warn("Warning " + std::to_string(i)); break;
            case 4: logger->error("Error " + std::to_string(i)); break;
            case 5: logger->critical("Critical " + std::to_string(i)); break;
        }
    }
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Trace"), std::string::npos);
    EXPECT_NE(content.find("Debug"), std::string::npos);
    EXPECT_NE(content.find("Info"), std::string::npos);
    EXPECT_NE(content.find("Warning"), std::string::npos);
    EXPECT_NE(content.find("Error"), std::string::npos);
    EXPECT_NE(content.find("Critical"), std::string::npos);
}

// ============================================================================
// Test 8.4.4: File System Edge Cases
// ============================================================================

TEST_F(LoggerAdvancedTest, LogToNestedDirectory) {
    std::filesystem::path testFile = "test_nested_dir/test.txt";
    
    // Create directory first
    std::filesystem::create_directories("test_nested_dir");
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    logger->info("Nested directory test");
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Nested directory test"), std::string::npos);
}

TEST_F(LoggerAdvancedTest, LargeLogFile) {
    std::filesystem::path testFile = "test_large_file.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    // Write enough to create a multi-megabyte file
    const int numLargeMessages = 10000;
    std::string largeMessage(1000, 'X'); // 1KB per message
    
    for (int i = 0; i < numLargeMessages; ++i) {
        logger->info(largeMessage + std::to_string(i));
    }
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Check file size is reasonably large (> 5MB)
    auto fileSize = std::filesystem::file_size(testFile);
    EXPECT_GT(fileSize, 5'000'000);
}

TEST_F(LoggerAdvancedTest, FileLoggerDestructorClosesFile) {
    std::filesystem::path testFile = "test_readonly.txt";
    
    {
        auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
        logger->info("Test message");
    } // Logger destroyed here
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Should be able to delete file (file is closed)
    EXPECT_NO_THROW(std::filesystem::remove(testFile));
}

// ============================================================================
// Test 8.4.5: Log Level Boundary Tests
// ============================================================================

TEST_F(LoggerAdvancedTest, AllLevelsWithTraceMinimum) {
    auto logger = LoggerFactory::createConsoleLogger(LogLevel::Trace, false);
    
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Trace));
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Debug));
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Info));
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Warning));
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Error));
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Critical));
}

TEST_F(LoggerAdvancedTest, AllLevelsWithCriticalMinimum) {
    auto logger = LoggerFactory::createConsoleLogger(LogLevel::Critical, false);
    
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Trace));
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Debug));
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Info));
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Warning));
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Error));
    EXPECT_TRUE(logger->isLevelEnabled(LogLevel::Critical));
}

TEST_F(LoggerAdvancedTest, DynamicLevelChangeFiltering) {
    std::filesystem::path testFile = "test_level_change.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
    
    logger->trace("Should not appear 1");
    logger->debug("Should not appear 2");
    logger->info("Should appear 1");
    
    logger->setLevel(LogLevel::Debug);
    
    logger->trace("Should not appear 3");
    logger->debug("Should appear 2");
    logger->info("Should appear 3");
    
    logger->setLevel(LogLevel::Error);
    
    logger->debug("Should not appear 4");
    logger->info("Should not appear 5");
    logger->error("Should appear 4");
    
    auto content = readFileContent(testFile);
    
    EXPECT_EQ(content.find("Should not appear 1"), std::string::npos);
    EXPECT_EQ(content.find("Should not appear 2"), std::string::npos);
    EXPECT_NE(content.find("Should appear 1"), std::string::npos);
    
    EXPECT_EQ(content.find("Should not appear 3"), std::string::npos);
    EXPECT_NE(content.find("Should appear 2"), std::string::npos);
    EXPECT_NE(content.find("Should appear 3"), std::string::npos);
    
    EXPECT_EQ(content.find("Should not appear 4"), std::string::npos);
    EXPECT_EQ(content.find("Should not appear 5"), std::string::npos);
    EXPECT_NE(content.find("Should appear 4"), std::string::npos);
}

// ============================================================================
// Test 8.4.6: LoggerFactory Edge Cases
// ============================================================================

TEST_F(LoggerAdvancedTest, NullLoggerDiscardsEverything) {
    auto logger = LoggerFactory::createNullLogger();
    
    // All logging calls should be no-ops
    EXPECT_NO_THROW({
        logger->trace("Discarded");
        logger->debug("Discarded");
        logger->info("Discarded");
        logger->warn("Discarded");
        logger->error("Discarded");
        logger->critical("Discarded");
    });
    
    // Verify NullLogger contract
    EXPECT_EQ(logger->getLevel(), LogLevel::Trace);
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Trace));
    EXPECT_FALSE(logger->isLevelEnabled(LogLevel::Critical));
}

TEST_F(LoggerAdvancedTest, DefaultLoggerCreation) {
    std::filesystem::path testFile = "default_test.txt";
    
    auto logger = LoggerFactory::createDefaultLogger(testFile, LogLevel::Debug, false);
    
    ASSERT_NE(logger, nullptr);
    EXPECT_EQ(logger->getLevel(), LogLevel::Debug);
    
    logger->debug("Default logger test");
    
    // Clean up
    if (std::filesystem::exists(testFile)) {
        std::filesystem::remove(testFile);
    }
}

TEST_F(LoggerAdvancedTest, ConsoleLoggerWithColors) {
    auto colorLogger = LoggerFactory::createConsoleLogger(LogLevel::Info, true);
    auto noColorLogger = LoggerFactory::createConsoleLogger(LogLevel::Info, false);
    
    ASSERT_NE(colorLogger, nullptr);
    ASSERT_NE(noColorLogger, nullptr);
    
    // Both should work
    EXPECT_NO_THROW(colorLogger->info("Color test"));
    EXPECT_NO_THROW(noColorLogger->info("No color test"));
}

// ============================================================================
// Test 8.4.7: Multiple Logger Instances
// ============================================================================

TEST_F(LoggerAdvancedTest, MultipleFileLoggersSameFile) {
    std::filesystem::path testFile = "shared_file.txt";
    
    {
        auto logger1 = LoggerFactory::createFileLogger(testFile, LogLevel::Info, false);
        logger1->info("Logger 1 message");
    }
    
    {
        auto logger2 = LoggerFactory::createFileLogger(testFile, LogLevel::Info, true);
        logger2->info("Logger 2 message");
    }
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Logger 1 message"), std::string::npos);
    EXPECT_NE(content.find("Logger 2 message"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(LoggerAdvancedTest, MultipleLoggersDifferentLevels) {
    auto traceLogger = LoggerFactory::createConsoleLogger(LogLevel::Trace, false);
    auto infoLogger = LoggerFactory::createConsoleLogger(LogLevel::Info, false);
    auto errorLogger = LoggerFactory::createConsoleLogger(LogLevel::Error, false);
    
    EXPECT_TRUE(traceLogger->isLevelEnabled(LogLevel::Trace));
    EXPECT_FALSE(infoLogger->isLevelEnabled(LogLevel::Trace));
    EXPECT_FALSE(errorLogger->isLevelEnabled(LogLevel::Trace));
    
    EXPECT_TRUE(traceLogger->isLevelEnabled(LogLevel::Info));
    EXPECT_TRUE(infoLogger->isLevelEnabled(LogLevel::Info));
    EXPECT_FALSE(errorLogger->isLevelEnabled(LogLevel::Info));
}

// ============================================================================
// Test 8.4.8: Stress Testing
// ============================================================================

TEST_F(LoggerAdvancedTest, AlternatingLoggersStressTest) {
    std::filesystem::path testFile = "stress_test.txt";
    
    auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Trace, false);
    
    for (int cycle = 0; cycle < 100; ++cycle) {
        logger->setLevel(static_cast<LogLevel>(cycle % 6));
        
        logger->trace("Cycle " + std::to_string(cycle));
        logger->debug("Cycle " + std::to_string(cycle));
        logger->info("Cycle " + std::to_string(cycle));
        logger->warn("Cycle " + std::to_string(cycle));
        logger->error("Cycle " + std::to_string(cycle));
        logger->critical("Cycle " + std::to_string(cycle));
    }
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    // Clean up
    std::filesystem::remove(testFile);
}

TEST_F(LoggerAdvancedTest, RepeatedLoggerCreationAndDestruction) {
    std::filesystem::path testFile = "repeated_test.txt";
    
    for (int i = 0; i < 50; ++i) {
        auto logger = LoggerFactory::createFileLogger(testFile, LogLevel::Info, true);
        logger->info("Iteration " + std::to_string(i));
    }
    
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    auto content = readFileContent(testFile);
    EXPECT_NE(content.find("Iteration 0"), std::string::npos);
    EXPECT_NE(content.find("Iteration 49"), std::string::npos);
    
    // Clean up
    std::filesystem::remove(testFile);
}
