/**
 * @file Application.cpp
 * @brief Implementation of core application management class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#include "application.hpp"

#include <stdexcept>

namespace truss::core {

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

bool Application::initialize() {
    if (m_initialized) {
        // Note: Logging removed - Application now dependencies-free
        // Callers should log initialization events if needed
        return true;
    }

    try {
        // Initialize application components here
        // For now, just mark as initialized
        m_initialized = true;

        return true;

    } catch (const std::exception& e) {
        // Note: Logging removed - caller should check return value
        return false;
    }
}

[[maybe_unused]] void Application::shutdown() {
    if (!m_initialized) {
        return;
    }

    // Cleanup application components here

    m_initialized = false;
}

}  // namespace truss::core
