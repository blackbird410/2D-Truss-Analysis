/**
 * @file Application.hpp
 * @brief Core application management class
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include <memory>
#include <string>

namespace truss::core {

/**
 * @brief Core application singleton class managing application lifecycle
 *
 * This class implements the singleton pattern to provide a central point
 * for application-wide configuration and state management.
 */
class Application {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the Application instance
     */
    static Application& getInstance();

    /**
     * @brief Initialize the application
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Shutdown the application
     */
    void shutdown();

    /**
     * @brief Check if application is initialized
     * @return true if initialized, false otherwise
     */
    [[maybe_unused]] bool isInitialized() const noexcept { return m_initialized; }

    /**
     * @brief Get application version
     * @return Version string
     */
    [[maybe_unused]] const std::string& getVersion() const noexcept { return m_version; }

    /**
     * @brief Get application name
     * @return Application name
     */
    const std::string& getName() const noexcept { return m_name; }

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

private:
    Application() = default;
    ~Application() = default;

    bool m_initialized{false};
    std::string m_version{"3.0.0"};
    std::string m_name{"TrussAnalysis2D"};
};

}  // namespace truss::core
