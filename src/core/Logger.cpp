/**
 * @file Logger.cpp
 * @brief Implementation of the logging system
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "Logger.hpp"
#include <iostream>
#include <sstream>

namespace truss::core {

// Static member definitions
std::ofstream Logger::s_logFile;
LogLevel Logger::s_currentLevel = LogLevel::Info;
bool Logger::s_initialized = false;
std::mutex Logger::s_logMutex;

bool Logger::initialize() {
    std::lock_guard<std::mutex> lock(s_logMutex);
    
    if (s_initialized) {
        return true;
    }
    
    try {
        // Open log file
        s_logFile.open("TrussAnalysis2D.log", std::ios::out | std::ios::app);
        if (!s_logFile.is_open()) {
            std::cerr << "Failed to open log file" << std::endl;
            return false;
        }
        
        s_initialized = true;
        log(LogLevel::Info, "Logging system initialized");
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(s_logMutex);
    
    if (!s_initialized) {
        return;
    }
    
    log(LogLevel::Info, "Shutting down logging system");
    
    if (s_logFile.is_open()) {
        s_logFile.close();
    }
    
    s_initialized = false;
}

void Logger::trace(const std::string& message) {
    log(LogLevel::Trace, message);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::Info, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::Error, message);
}

void Logger::critical(const std::string& message) {
    log(LogLevel::Critical, message);
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    s_currentLevel = level;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_logMutex);
    
    // Check if log level is sufficient
    if (level < s_currentLevel) {
        return;
    }
    
    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = levelToString(level);
    
    // Format: [TIMESTAMP] [LEVEL] MESSAGE
    std::string logLine = "[" + timestamp + "] [" + levelStr + "] " + message;
    
    // Output to console
    if (level >= LogLevel::Warning) {
        std::cerr << logLine << std::endl;
    } else {
        std::cout << logLine << std::endl;
    }
    
    // Output to file if available
    if (s_initialized && s_logFile.is_open()) {
        s_logFile << logLine << std::endl;
        s_logFile.flush();
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO ";
        case LogLevel::Warning:  return "WARN ";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT ";
        default:                 return "UNKN ";
    }
}

} // namespace truss::core
