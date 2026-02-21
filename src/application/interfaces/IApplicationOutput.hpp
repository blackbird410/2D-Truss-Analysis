/**
 * @file IApplicationOutput.hpp
 * @brief Application-level output abstraction
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Defines output contract for CLI and other application frontends.
 * Implementations may target console, file, network, or other destinations.
 *
 * Responsibilities:
 * - Define semantic output levels (info, success, error, warn)
 * - Provide message output interface
 * - NO formatting logic (handled by Presenter)
 * - NO I/O implementation (handled by Infrastructure)
 *
 * Clean Architecture Compliance:
 * - Interface owned by Application layer
 * - Implemented by Infrastructure layer
 * - Consumed by CLI layer
 */

#pragma once

#include <string>

namespace truss::application::interfaces {

/**
 * @brief Output abstraction for application messages
 *
 * Provides semantic output methods for different message types.
 * Implementations handle routing to appropriate destinations
 * (console, file, network) with appropriate formatting.
 */
class IApplicationOutput {
public:
    virtual ~IApplicationOutput() = default;

    /**
     * @brief Output informational message
     * @param message The message content to display
     *
     * Used for general information, status updates, and neutral content.
     * Implementations may route to stdout with optional formatting.
     */
    virtual void info(const std::string& message) = 0;

    /**
     * @brief Output success message
     * @param message The message content to display
     *
     * Used for successful operations, confirmations, and positive feedback.
     * Implementations may add visual indicators (colors, icons, etc.).
     */
    virtual void success(const std::string& message) = 0;

    /**
     * @brief Output error message
     * @param message The message content to display
     *
     * Used for errors, failures, and critical issues requiring attention.
     * Implementations may route to stderr with error-level formatting.
     */
    virtual void error(const std::string& message) = 0;

    /**
     * @brief Output warning message
     * @param message The message content to display
     *
     * Used for warnings, potential issues, and cautionary information.
     * Implementations may use distinctive formatting to draw attention.
     */
    virtual void warn(const std::string& message) = 0;
};

}  // namespace truss::application::interfaces
