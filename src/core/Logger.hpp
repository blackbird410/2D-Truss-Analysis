/**
 * @file Logger.hpp
 * @brief Modern logging system interface
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
 */

#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace truss::core {

/**
 * @brief Logging levels enum
 */
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

/**
 * @brief Simple logging system for the application
 * 
 * This class provides a clean interface for logging throughout the application.
 * Uses standard C++ streams for output.
 */
class Logger {
public:
    /**
     * @brief Initialize the logging system
     * @return true if initialization was successful, false otherwise
     */
    static bool initialize();
    
    /**
     * @brief Shutdown the logging system
     */
    static void shutdown();
    
    /**
     * @brief Log a trace message
     */
    static void trace(const std::string& message);
    
    /**
     * @brief Log a debug message
     */
    static void debug(const std::string& message);
    
    /**
     * @brief Log an info message
     */
    static void info(const std::string& message);
    
    /**
     * @brief Log a warning message
     */
    static void warn(const std::string& message);
    
    /**
     * @brief Log an error message
     */
    static void error(const std::string& message);
    
    /**
     * @brief Log a critical message
     */
    static void critical(const std::string& message);
    
    /**
     * @brief Set the logging level
     * @param level New logging level
     */
    static void setLevel(LogLevel level);
    
private:
    /**
     * @brief Core logging function
     * @param level Log level
     * @param message Message to log
     */
    static void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Get current timestamp string
     */
    static std::string getCurrentTimestamp();
    
    /**
     * @brief Convert log level to string
     */
    static std::string levelToString(LogLevel level);
    
    static std::ofstream s_logFile;
    static LogLevel s_currentLevel;
    static bool s_initialized;
    static std::mutex s_logMutex;
};

} // namespace truss::core
