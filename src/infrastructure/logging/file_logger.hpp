/**
 * @file file_logger.hpp
 * @brief File logger implementation
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-09
 */

#pragma once

#include "logger.hpp"
#include <fstream>
#include <mutex>
#include <filesystem>

namespace truss::infrastructure::logging {

/**
 * @brief Logger that outputs to a file
 * 
 * Features:
 * - Thread-safe file writing
 * - Automatic file creation
 * - Append mode (preserves existing logs)
 * - Timestamps on messages
 * - Automatic flushing for critical errors
 * 
 * Thread Safety: Fully thread-safe via mutex
 */
class FileLogger : public ILogger {
public:
    /**
     * @brief Construct file logger
     * @param filePath Path to log file
     * @param minLevel Minimum log level (default: Info)
     * @param append Append to existing file (default: true)
     * @throws std::runtime_error if file cannot be opened
     */
    explicit FileLogger(
        const std::filesystem::path& filePath,
        LogLevel minLevel = LogLevel::Info,
        bool append = true
    );
    
    ~FileLogger() override;
    
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
    
    /**
     * @brief Flush pending writes to disk
     * 
     * Automatically called on errors/critical messages.
     * Can be called manually to ensure data is written.
     */
    void flush();
    
    /**
     * @brief Check if log file is open
     * @return true if file is writable
     */
    bool isOpen() const;

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
    std::string getCurrentTimestamp() const;
    
    /**
     * @brief Get log level string
     * @param level Severity level
     * @return Formatted level string (e.g., "[INFO ]")
     */
    std::string getLevelString(LogLevel level) const;
    
    std::filesystem::path m_filePath; ///< Log file path
    std::ofstream m_file;             ///< Output file stream
    LogLevel m_minLevel;              ///< Minimum log level
    mutable std::mutex m_mutex;       ///< Thread safety mutex
};

} // namespace truss::infrastructure::logging
