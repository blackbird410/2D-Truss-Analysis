/**
 * @file console_logger.cpp
 * @brief Logs messages to standard output with level filtering.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "console_logger.hpp"

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <sstream>

namespace truss::infrastructure::logging {

ConsoleLogger::ConsoleLogger(LogLevel minLevel, bool useColors)
    : m_minLevel(minLevel), m_useColors(useColors) {}

void ConsoleLogger::trace(const std::string& message) {
    log(LogLevel::Trace, message);
}

void ConsoleLogger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void ConsoleLogger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void ConsoleLogger::warn(const std::string& message) {
    log(LogLevel::Warning, message);
}

void ConsoleLogger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void ConsoleLogger::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

void ConsoleLogger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

LogLevel ConsoleLogger::getLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

bool ConsoleLogger::isLevelEnabled(LogLevel level) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(level) >= static_cast<int>(m_minLevel);
}

void ConsoleLogger::log(LogLevel level, const std::string& message) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check level filter (duplicates isLevelEnabled logic to avoid deadlock,
    // since isLevelEnabled() also acquires m_mutex)
    if (static_cast<int>(level) < static_cast<int>(m_minLevel)) {
        return;
    }

    // Build log line
    std::ostringstream oss;
    oss << "[" << getCurrentTimestamp() << "] " << getColorCode(level) << getLevelString(level)
        << getResetCode() << " " << message;

    std::string logLine = oss.str();

    // Output to appropriate stream
    std::FILE* stream = (level >= LogLevel::Error) ? stderr : stdout;
    std::fwrite(logLine.c_str(), 1, logLine.size(), stream);
    std::fwrite("\n", 1, 1, stream);
    std::fflush(stream);
}

std::string ConsoleLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    // Use thread-safe time conversion
    std::tm timeInfo;
    localtime_r(&time, &timeInfo);

    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string ConsoleLogger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
            return "[TRACE]";
        case LogLevel::Debug:
            return "[DEBUG]";
        case LogLevel::Info:
            return "[INFO ]";
        case LogLevel::Warning:
            return "[WARN ]";
        case LogLevel::Error:
            return "[ERROR]";
        case LogLevel::Critical:
            return "[CRIT ]";
        default:
            return "[UNKN ]";
    }
}

std::string ConsoleLogger::getColorCode(LogLevel level) const {
    if (!m_useColors) {
        return "";
    }

    switch (level) {
        case LogLevel::Trace:
            return "\033[37m";  // White
        case LogLevel::Debug:
            return "\033[36m";  // Cyan
        case LogLevel::Info:
            return "\033[32m";  // Green
        case LogLevel::Warning:
            return "\033[33m";  // Yellow
        case LogLevel::Error:
            return "\033[31m";  // Red
        case LogLevel::Critical:
            return "\033[1;31m";  // Bold Red
        default:
            return "";
    }
}

std::string ConsoleLogger::getResetCode() const {
    return m_useColors ? "\033[0m" : "";
}

}  // namespace truss::infrastructure::logging
