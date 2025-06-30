/**
 * @file Application.cpp
 * @brief Implementation of core application management class
 * @author Civil Engineering Software Solutions
 * @version 2.0.0
 */

#include "Application.hpp"
#include "Logger.hpp"
#include <stdexcept>

namespace truss::core {

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

bool Application::initialize() {
    if (m_initialized) {
        Logger::warn("Application already initialized");
        return true;
    }
    
    try {
        Logger::info("Initializing application: " + m_name + " v" + m_version);
        
        // Initialize application components here
        // For now, just mark as initialized
        m_initialized = true;
        
        Logger::info("Application initialization completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Application initialization failed: " + std::string(e.what()));
        return false;
    }
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    Logger::info("Shutting down application");
    
    // Cleanup application components here
    
    m_initialized = false;
    Logger::info("Application shutdown completed");
}

} // namespace truss::core
