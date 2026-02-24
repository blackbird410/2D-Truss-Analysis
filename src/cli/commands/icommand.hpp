/**
 * @file icommand.hpp
 * @brief Command interface for CLI.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Defines the contract for all CLI commands.
 * Commands delegate business logic to Application services.
 */

#pragma once

#include <string>

namespace truss::cli::commands {

/**
 * @brief Interface for all CLI commands
 *
 * Each command represents a single user-facing CLI operation.
 * Commands delegate to Application services and presenters.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @brief Execute the command
     * @return Exit code (0 = success, non-zero = error)
     */
    virtual int execute() = 0;

    /**
     * @brief Get command name for registration
     * @return Command name (e.g., "example", "analyze", "help")
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get command description for help text
     * @return Brief description of command purpose
     */
    virtual std::string getDescription() const = 0;
};

}  // namespace truss::cli::commands
