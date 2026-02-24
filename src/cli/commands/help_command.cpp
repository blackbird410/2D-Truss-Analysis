/**
 * @file help_command.cpp
 * @brief Implements the help command.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "help_command.hpp"

#include <sstream>

namespace truss::cli::commands {

HelpCommand::HelpCommand(truss::cli::presenters::ConsolePresenter& presenter,
                         const std::vector<ICommand*>& commands)
    : m_presenter(presenter), m_commands(commands) {}

int HelpCommand::execute() {
    displayUsage();
    displayCommands();
    return 0;
}

void HelpCommand::displayUsage() const {
    m_presenter.displayInfo("Usage:");
    m_presenter.displayInfo("  TrussAnalysisCLI [options] <command>");
    m_presenter.displayInfo("");
    m_presenter.displayInfo("Options:");
    m_presenter.displayInfo("  -h, --help     Show this help message");
    m_presenter.displayInfo("  -v, --verbose  Enable verbose output");
    m_presenter.displayInfo("");
}

void HelpCommand::displayCommands() const {
    m_presenter.displayInfo("Available Commands:");

    for (const auto* cmd : m_commands) {
        if (cmd) {
            std::ostringstream oss;
            oss << "  " << cmd->getName() << "\t\t" << cmd->getDescription();
            m_presenter.displayInfo(oss.str());
        }
    }

    m_presenter.displayInfo("");
    m_presenter.displayInfo("Examples:");
    m_presenter.displayInfo("  TrussAnalysisCLI example        # Run example analysis");
    m_presenter.displayInfo("  TrussAnalysisCLI help           # Show this help");
    m_presenter.displayInfo("  TrussAnalysisCLI -v example     # Run with verbose output");
    m_presenter.displayInfo("");
}

}  // namespace truss::cli::commands
