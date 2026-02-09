/**
 * @file logger_factory.hpp
 * @brief Factory for creating logger instances
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-09
 */

#pragma once

#include "logger.hpp"
#include "console_logger.hpp"
#include "file_logger.hpp"
#include <memory>
#include <filesystem>
#include <vector>

namespace truss::infrastructure::logging {

/**
 * @brief Factory for creating concrete logger instances
 * 
 * Design Pattern: Factory Method
 * - Encapsulates logger instantiation logic
 * - Client code depends only on ILogger interface
 * - Supports multiple logger types and combinations
 * 
 * Usage:
 * @code
 *   // Console only
 *   auto logger = LoggerFactory::createConsoleLogger();
 *   
 *   // File only
 *   auto logger = LoggerFactory::createFileLogger("app.log");
 *   
 *   // Both console and file
 *   auto logger = LoggerFactory::createDefaultLogger("app.log");
 * @endcode
 */
class LoggerFactory {
public:
    /**
     * @brief Create a console-only logger
     * 
     * @param minLevel Minimum log level (default: Info)
     * @param useColors Enable ANSI colors (default: true)
     * @return Shared pointer to console logger
     */
    static LoggerPtr createConsoleLogger(
        LogLevel minLevel = LogLevel::Info,
        bool useColors = true
    );
    
    /**
     * @brief Create a file-only logger
     * 
     * @param filePath Path to log file
     * @param minLevel Minimum log level (default: Info)
     * @param append Append to existing file (default: true)
     * @return Shared pointer to file logger
     * @throws std::runtime_error if file cannot be created
     */
    static LoggerPtr createFileLogger(
        const std::filesystem::path& filePath,
        LogLevel minLevel = LogLevel::Info,
        bool append = true
    );
    
    /**
     * @brief Create a dual logger (console + file)
     * 
     * Writes to both console and file simultaneously.
     * This is the recommended logger for most applications.
     * 
     * @param filePath Path to log file
     * @param minLevel Minimum log level (default: Info)
     * @param useColors Enable console colors (default: true)
     * @return Shared pointer to composite logger
     * @throws std::runtime_error if file cannot be created
     */
    static LoggerPtr createDefaultLogger(
        const std::filesystem::path& filePath = "TrussAnalysis2D.log",
        LogLevel minLevel = LogLevel::Info,
        bool useColors = true
    );
    
    /**
     * @brief Create a null logger (discards all messages)
     * 
     * Useful for testing or disabling logging.
     * 
     * @return Shared pointer to null logger
     */
    static LoggerPtr createNullLogger();

private:
    // Factory is a utility class, no instantiation
    LoggerFactory() = delete;
    
    /**
     * @brief Internal composite logger that writes to multiple loggers
     */
    class CompositeLogger : public ILogger {
    public:
        explicit CompositeLogger(std::vector<LoggerPtr> loggers);
        
        void trace(const std::string& message) override;
        void debug(const std::string& message) override;
        void info(const std::string& message) override;
        void warn(const std::string& message) override;
        void error(const std::string& message) override;
        void critical(const std::string& message) override;
        
        void setLevel(LogLevel level) override;
        LogLevel getLevel() const override;
        bool isLevelEnabled(LogLevel level) const override;
        
    private:
        std::vector<LoggerPtr> m_loggers;
        LogLevel m_minLevel;
    };
    
    /**
     * @brief Internal null logger implementation
     */
    class NullLogger : public ILogger {
    public:
        void trace(const std::string&) override {}
        void debug(const std::string&) override {}
        void info(const std::string&) override {}
        void warn(const std::string&) override {}
        void error(const std::string&) override {}
        void critical(const std::string&) override {}
        
        void setLevel(LogLevel) override {}
        LogLevel getLevel() const override { return LogLevel::Critical; }
        bool isLevelEnabled(LogLevel) const override { return false; }
    };
};

} // namespace truss::infrastructure::logging
