/**
 * @file logger_factory.hpp
 * @brief Factory for creating logger instances
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-09
 */

#pragma once

#include "console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
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
    static LoggerPtr createConsoleLogger(LogLevel minLevel = LogLevel::Info, bool useColors = true);

    /**
     * @brief Create a file-only logger
     *
     * @param filePath Path to log file
     * @param minLevel Minimum log level (default: Info)
     * @param append Append to existing file (default: true)
     * @return Shared pointer to file logger
     * @throws std::runtime_error if file cannot be created
     */
    static LoggerPtr createFileLogger(const std::filesystem::path& filePath,
                                      LogLevel minLevel = LogLevel::Info,
                                      bool append = true);

    /**
     * @brief Create a dual logger (console + file)
     *
     * Writes to both console and file simultaneously when possible.
     * This is the recommended logger for most applications.
     *
     * The factory will attempt to create a file logger using the provided
     * path. If the log file cannot be created or opened, the method
     * falls back to a console-only logger without throwing an exception.
     * An error message is logged to the console when fallback occurs.
     *
     * @param filePath Path to log file (default: "TrussAnalysis2D.log")
     * @param minLevel Minimum log level (default: Info)
     * @param useColors Enable console colors (default: true)
     * @return Shared pointer to composite logger (or console-only logger
     *         if file logging is unavailable)
     */
    static LoggerPtr
    createDefaultLogger(const std::filesystem::path& filePath = "TrussAnalysis2D.log",
                        LogLevel minLevel = LogLevel::Info,
                        bool useColors = true);

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
        mutable std::mutex m_mutex;  ///< Thread safety mutex
    };

    /**
     * @brief Internal null logger implementation
     *
     * Discards all log messages. The getLevel() method returns Trace level
     * (most permissive) because NullLogger accepts all messages for processing,
     * even though it ultimately discards them. The isLevelEnabled() method
     * always returns false to signal that the NullLogger doesn't actually
     * produce any output.
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
        // Return Trace to indicate most permissive level (accepts all messages)
        LogLevel getLevel() const override { return LogLevel::Trace; }
        // Always return false since NullLogger discards all messages
        bool isLevelEnabled(LogLevel) const override { return false; }
    };
};

}  // namespace truss::infrastructure::logging
