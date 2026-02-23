/**
 * @file ConsoleOutputAdapter.hpp
 * @brief Adapter bridging Application output to Infrastructure logging
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * Implements IApplicationOutput by delegating to Infrastructure logging system.
 * Provides semantic mapping from Application output methods to Logger levels.
 *
 * Design Pattern: Adapter
 * - Target: IApplicationOutput (Application interface)
 * - Adaptee: ILogger (Infrastructure interface)
 * - Adapter: ConsoleOutputAdapter (this class)
 *
 * Responsibilities:
 * - Implement IApplicationOutput interface
 * - Delegate to ILogger implementation
 * - Map semantic methods (success, error) to log levels
 * - NO formatting logic (passed through from Presenter)
 */

#pragma once

#include "../../application/interfaces/iapplication_output.hpp"
#include "../logging/logger.hpp"

#include <string>

namespace truss::infrastructure::adapters {

/**
 * @brief Console output adapter using Infrastructure logging
 *
 * Bridges Application-level output abstraction to Infrastructure
 * logging implementation. Enables CLI to output via existing
 * ConsoleLogger with thread safety, timestamps, and colors.
 */
class ConsoleOutputAdapter : public application::interfaces::IApplicationOutput {
public:
    /**
     * @brief Construct adapter with logger dependency
     * @param logger Reference to Infrastructure logger implementation
     *
     * Logger must outlive this adapter. Typically injected from
     * composition root (main_app.cpp).
     */
    explicit ConsoleOutputAdapter(logging::ILogger& logger);

    ~ConsoleOutputAdapter() override = default;

    // IApplicationOutput interface implementation
    void info(const std::string& message) override;
    void success(const std::string& message) override;
    void error(const std::string& message) override;
    void warn(const std::string& message) override;

private:
    logging::ILogger& m_logger;
};

}  // namespace truss::infrastructure::adapters
