/**
 * @file application.cpp
 * @brief Core application lifecycle management.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
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
