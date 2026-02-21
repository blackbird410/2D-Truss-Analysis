/**
 * @file logger_factory.cpp
 * @brief Factory for creating logger instances
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * @date 2026-02-09
 */

#include "logger_factory.hpp"

namespace truss::infrastructure::logging {

// ===== Public Factory Methods =====

LoggerPtr LoggerFactory::createConsoleLogger(LogLevel minLevel, bool useColors) {
    return std::make_shared<ConsoleLogger>(minLevel, useColors);
}

LoggerPtr LoggerFactory::createFileLogger(const std::filesystem::path& filePath,
                                          LogLevel minLevel,
                                          bool append) {
    return std::make_shared<FileLogger>(filePath, minLevel, append);
}

LoggerPtr LoggerFactory::createDefaultLogger(const std::filesystem::path& filePath,
                                             LogLevel minLevel,
                                             bool useColors) {
    std::vector<LoggerPtr> loggers;

    // Add console logger
    loggers.push_back(createConsoleLogger(minLevel, useColors));

    // Add file logger (catch exceptions to avoid breaking app if file fails)
    try {
        loggers.push_back(createFileLogger(filePath, minLevel, true));
    } catch (const std::runtime_error& e) {
        // Log error to console logger only
        loggers[0]->error("Failed to create file logger: " + std::string(e.what()));
    }

    return std::make_shared<CompositeLogger>(std::move(loggers));
}

[[maybe_unused]] LoggerPtr LoggerFactory::createNullLogger() {
    return std::make_shared<NullLogger>();
}

// ===== CompositeLogger Implementation =====

LoggerFactory::CompositeLogger::CompositeLogger(std::vector<LoggerPtr> loggers)
    : m_loggers(std::move(loggers)), m_minLevel(LogLevel::Critical) {
    // Set minimum level to the most permissive (lowest enum value) among child loggers
    for (const auto& logger : m_loggers) {
        if (static_cast<int>(logger->getLevel()) < static_cast<int>(m_minLevel)) {
            m_minLevel = logger->getLevel();
        }
    }
}

void LoggerFactory::CompositeLogger::trace(const std::string& message) {
    for (auto& logger : m_loggers) {
        logger->trace(message);
    }
}

void LoggerFactory::CompositeLogger::debug(const std::string& message) {
    for (auto& logger : m_loggers) {
        logger->debug(message);
    }
}

void LoggerFactory::CompositeLogger::info(const std::string& message) {
    for (auto& logger : m_loggers) {
        logger->info(message);
    }
}

void LoggerFactory::CompositeLogger::warn(const std::string& message) {
    for (auto& logger : m_loggers) {
        logger->warn(message);
    }
}

void LoggerFactory::CompositeLogger::error(const std::string& message) {
    for (auto& logger : m_loggers) {
        logger->error(message);
    }
}

void LoggerFactory::CompositeLogger::critical(const std::string& message) {
    for (auto& logger : m_loggers) {
        logger->critical(message);
    }
}

void LoggerFactory::CompositeLogger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
    for (auto& logger : m_loggers) {
        logger->setLevel(level);
    }
}

LogLevel LoggerFactory::CompositeLogger::getLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

bool LoggerFactory::CompositeLogger::isLevelEnabled(LogLevel level) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(level) >= static_cast<int>(m_minLevel);
}

}  // namespace truss::infrastructure::logging
