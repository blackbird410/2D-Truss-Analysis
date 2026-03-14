/**
 * @file file_logger.cpp
 * @brief Logs messages to rotating log files.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "file_logger.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace truss::infrastructure::logging {

FileLogger::FileLogger(const std::filesystem::path& filePath, LogLevel minLevel, bool append)
    : m_filePath(filePath), m_file(), m_minLevel(minLevel), m_mutex() {
    // Open file with appropriate mode
    auto mode = std::ios::out;
    if (append) {
        mode |= std::ios::app;
    } else {
        mode |= std::ios::trunc;
    }

    m_file.open(m_filePath, mode);

    if (!m_file.is_open()) {
        throw std::runtime_error("Failed to open log file: " + m_filePath.string());
    }

    // Log initialization message (lifecycle messages bypass level filtering)
    std::string timestamp = getCurrentTimestamp();
    m_file << "[" << timestamp
           << "] [INFO ] Logger initialized (file: " << m_filePath.filename().string() << ")"
           << std::endl;
    m_file.flush();
}

FileLogger::~FileLogger() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        // Log shutdown message (lifecycle messages bypass level filtering)
        std::string timestamp = getCurrentTimestamp();
        m_file << "[" << timestamp << "] [INFO ] Logger shutting down" << std::endl;
        m_file.flush();
        m_file.close();
    }
}

void FileLogger::trace(const std::string& message) {
    log(LogLevel::Trace, message);
}

void FileLogger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void FileLogger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void FileLogger::warn(const std::string& message) {
    log(LogLevel::Warning, message);
}

void FileLogger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void FileLogger::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

void FileLogger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

LogLevel FileLogger::getLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

bool FileLogger::isLevelEnabled(LogLevel level) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(level) >= static_cast<int>(m_minLevel);
}

void FileLogger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file.flush();
    }
}

[[maybe_unused]] bool FileLogger::isOpen() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_file.is_open();
}

void FileLogger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check level filter (duplicates isLevelEnabled logic to avoid deadlock,
    // since isLevelEnabled() also acquires m_mutex)
    if (static_cast<int>(level) < static_cast<int>(m_minLevel)) {
        return;
    }

    if (!m_file.is_open()) {
        return;  // Silently fail if file is closed
    }

    // Build log line
    std::ostringstream oss;
    oss << "[" << getCurrentTimestamp() << "] " << getLevelString(level) << " " << message;

    // Write to file
    m_file << oss.str() << std::endl;

    // Flush immediately for errors/critical
    if (level >= LogLevel::Error) {
        m_file.flush();
    }
}

std::string FileLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    // Use thread-safe time conversion
    std::tm timeInfo;
    localtime_r(&time, &timeInfo);

    std::ostringstream oss;
    oss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string FileLogger::getLevelString(LogLevel level) {
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

}  // namespace truss::infrastructure::logging
