/**
 * @file console_logger.hpp
 * @brief Console logger implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-09
 */

#pragma once

#include "logger.hpp"

#include <mutex>
#include <sstream>

namespace truss::infrastructure::logging {

/**
 * @brief Logger that outputs to console (stdout/stderr)
 *
 * Features:
 * - Thread-safe output
 * - Timestamps on messages
 * - Color-coded severity levels (if terminal supports ANSI colors)
 * - Errors/Critical go to stderr, others to stdout
 *
 * Thread Safety: Fully thread-safe via mutex
 */
class ConsoleLogger : public ILogger {
public:
    /**
     * @brief Construct console logger
     * @param minLevel Minimum log level (default: Info)
     * @param useColors Enable ANSI color codes (default: true)
     */
    explicit ConsoleLogger(LogLevel minLevel = LogLevel::Info, bool useColors = true);

    ~ConsoleLogger() override = default;

    // ILogger interface implementation
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
    /**
     * @brief Core logging implementation
     * @param level Severity level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);

    /**
     * @brief Get current timestamp string
     * @return ISO 8601 timestamp (YYYY-MM-DD HH:MM:SS)
     */
    static std::string getCurrentTimestamp();

    /**
     * @brief Get log level string with optional color codes
     * @param level Severity level
     * @return Formatted level string (e.g., "[INFO ]")
     */
    static std::string getLevelString(LogLevel level);

    /**
     * @brief Get ANSI color code for log level
     * @param level Severity level
     * @return ANSI escape sequence (empty if colors disabled)
     */
    std::string getColorCode(LogLevel level) const;

    /**
     * @brief Get ANSI color reset code
     * @return ANSI reset sequence (empty if colors disabled)
     */
    std::string getResetCode() const;

    LogLevel m_minLevel;         ///< Minimum log level
    bool m_useColors;            ///< Use ANSI color codes
    mutable std::mutex m_mutex;  ///< Thread safety mutex
};

}  // namespace truss::infrastructure::logging
