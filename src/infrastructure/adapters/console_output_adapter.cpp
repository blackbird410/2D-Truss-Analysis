/**
 * @file ConsoleOutputAdapter.cpp
 * @brief Implementation of console output adapter
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
