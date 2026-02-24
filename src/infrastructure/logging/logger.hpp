/**
 * @file logger.hpp
 * @brief Logger interface (Strategy pattern).
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#pragma once

#include <memory>
#include <string>

namespace truss::infrastructure::logging {

/**
 * @brief Logging severity levels
 *
 * Ordered from least to most severe.
 * Loggers can filter messages based on configured minimum level.
 */
enum class LogLevel {
    Trace,    ///< Detailed trace information (function entry/exit)
    Debug,    ///< Debug information for developers
    Info,     ///< General informational messages
    Warning,  ///< Warning messages (non-critical issues)
    Error,    ///< Error messages (operation failed)
    Critical  ///< Critical errors (system failure)
};

/**
 * @brief Abstract interface for logging subsystem.
 *
 * Defines the contract for all concrete logger implementations.
 * Supports multiple logging targets (console, file, etc.) and severity levels.
 *
 * Design Pattern: Strategy
 * - Context: LoggerFactory
 * - Strategy: ILogger (this interface)
 * - Concrete Strategies: ConsoleLogger, FileLogger, CompositeLogger
 *
 * Thread Safety: Concrete implementations MUST be thread-safe
 *
 * Usage:
 * @code
 *   auto logger = LoggerFactory::createConsoleLogger();
 *   logger->info("Application started");
 *   logger->error("Operation failed: " + errorMsg);
 * @endcode
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    /**
     * @brief Log a trace message
     * @param message Message to log
     */
    virtual void trace(const std::string& message) = 0;

    /**
     * @brief Log a debug message
     * @param message Message to log
     */
    virtual void debug(const std::string& message) = 0;

    /**
     * @brief Log an info message
     * @param message Message to log
     */
    virtual void info(const std::string& message) = 0;

    /**
     * @brief Log a warning message
     * @param message Message to log
     */
    virtual void warn(const std::string& message) = 0;

    /**
     * @brief Log an error message
     * @param message Message to log
     */
    virtual void error(const std::string& message) = 0;

    /**
     * @brief Log a critical error message
     * @param message Message to log
     */
    virtual void critical(const std::string& message) = 0;

    /**
     * @brief Set minimum log level
     *
     * Messages below this level will be filtered out.
     *
     * @param level Minimum severity level to log
     */
    virtual void setLevel(LogLevel level) = 0;

    /**
     * @brief Get current minimum log level
     * @return Current minimum log level
     */
    virtual LogLevel getLevel() const = 0;

    /**
     * @brief Check if a log level is enabled
     * @param level Level to check
     * @return true if messages at this level will be logged
     */
    virtual bool isLevelEnabled(LogLevel level) const = 0;
};

/**
 * @brief Shared pointer to logger (recommended for client code)
 */
using LoggerPtr = std::shared_ptr<ILogger>;

}  // namespace truss::infrastructure::logging
