/**
 * @file HelpCommand.cpp
 * @brief Implementation of help command
 */

#include "HelpCommand.hpp"
#include <iostream>

namespace truss::cli::commands {

HelpCommand::HelpCommand(
    truss::cli::presenters::ConsolePresenter& presenter,
    const std::vector<ICommand*>& commands
) : m_presenter(presenter),
    m_commands(commands)
{}

int HelpCommand::execute() {
    displayUsage();
    displayCommands();
    return 0;
}

void HelpCommand::displayUsage() const {
    std::cout << "Usage:\n";
    std::cout << "  TrussAnalysisCLI [options] <command>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --verbose  Enable verbose output\n\n";
}

void HelpCommand::displayCommands() const {
    std::cout << "Available Commands:\n";
    
    for (const auto* cmd : m_commands) {
        if (cmd) {
            std::cout << "  " << cmd->getName() << "\t\t" 
                     << cmd->getDescription() << "\n";
        }
    }
    
    std::cout << "\nExamples:\n";
    std::cout << "  TrussAnalysisCLI example        # Run example analysis\n";
    std::cout << "  TrussAnalysisCLI help           # Show this help\n";
    std::cout << "  TrussAnalysisCLI -v example     # Run with verbose output\n\n";
}

} // namespace truss::cli::commands
