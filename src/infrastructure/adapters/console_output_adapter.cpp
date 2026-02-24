/**
 * @file console_output_adapter.cpp
 * @brief Implements the console output adapter.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "console_output_adapter.hpp"

#include <string>

namespace truss::infrastructure::adapters {

ConsoleOutputAdapter::ConsoleOutputAdapter(logging::ILogger& logger) : m_logger(logger) {}

void ConsoleOutputAdapter::info(const std::string& message) {
    m_logger.info(message);
}

void ConsoleOutputAdapter::success(const std::string& message) {
    m_logger.info(message);
}

void ConsoleOutputAdapter::error(const std::string& message) {
    m_logger.error(message);
}

void ConsoleOutputAdapter::warn(const std::string& message) {
    m_logger.warn(message);
}

}  // namespace truss::infrastructure::adapters
