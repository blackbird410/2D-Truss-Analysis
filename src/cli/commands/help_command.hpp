/**
 * @file HelpCommand.hpp
 * @brief Help command for displaying CLI usage information
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 */

#pragma once

#include "../presenters/console_presenter.hpp"
#include "icommand.hpp"

#include <memory>
#include <vector>

namespace truss::cli::commands {

/**
 * @brief Command to display help information
 *
 * Dynamically generates help text from registered commands.
 */
class HelpCommand : public ICommand {
public:
    /**
     * @brief Constructor
     * @param presenter Injected console presenter
     * @param commands List of available commands for dynamic help
     */
    HelpCommand(truss::cli::presenters::ConsolePresenter& presenter,
                const std::vector<ICommand*>& commands);

    ~HelpCommand() override = default;

    // ICommand interface implementation
    int execute() override;
    std::string getName() const override { return "help"; }
    std::string getDescription() const override { return "Display this help information"; }

private:
    truss::cli::presenters::ConsolePresenter& m_presenter;
    const std::vector<ICommand*>& m_commands;

    void displayUsage() const;
    void displayCommands() const;
};

}  // namespace truss::cli::commands
