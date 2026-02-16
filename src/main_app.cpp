/**
 * @file main_app.cpp
 * @brief CLI entry point for 2D Truss Analysis
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Implements Command Pattern with dependency injection.
 * Wires Application services to CLI commands and dispatches execution.
 */

// Application Layer includes
#include "application/TrussApplicationService.hpp"
#include "application/AnalysisApplicationService.hpp"

// CLI Layer includes
#include "cli/ArgumentParser.hpp"
#include "cli/commands/ICommand.hpp"
#include "cli/commands/ExampleCommand.hpp"
#include "cli/commands/HelpCommand.hpp"
#include "cli/presenters/ConsolePresenter.hpp"

#include <iostream>
#include <memory>
#include <map>
#include <vector>

/**
 * @brief Execute CLI command
 * 
 * @param args Parsed command line arguments
 * @param commands Map of registered commands
 * @return Exit code (0 = success)
 */
int executeCommand(
    const truss::cli::ParsedArgs& args,
    std::map<std::string, std::unique_ptr<truss::cli::commands::ICommand>>& commands
) {
    auto it = commands.find(args.commandName);
    if (it == commands.end()) {
        std::cerr << "ERROR: Unknown command '" << args.commandName << "'\n";
        std::cerr << "Run 'TrussAnalysisCLI help' for available commands.\n";
        return 1;
    }
    
    return it->second->execute();
}

/**
 * @brief CLI entry point
 * 
 * Wires dependencies and dispatches command execution:
 * 1. Creates Application services and presenters
 * 2. Parses command-line arguments
 * 3. Registers commands with dependency injection
 * 4. Routes to appropriate command and executes
 */
int main(int argc, char* argv[]) {
    // Create presenter and display header
    truss::cli::presenters::ConsolePresenter presenter;
    presenter.displayHeader();
    
    // Create Application services
    truss::application::TrussApplicationService trussService;
    truss::application::AnalysisApplicationService analysisService;
    
    // Parse command-line arguments
    truss::cli::ArgumentParser parser;
    truss::cli::ParsedArgs args = parser.parse(argc, argv);
    
    // Create command registry
    std::map<std::string, std::unique_ptr<truss::cli::commands::ICommand>> commands;
    std::vector<truss::cli::commands::ICommand*> commandPtrs;
    
    // Register commands with dependency injection
    
    // ExampleCommand
    auto exampleCmd = std::make_unique<truss::cli::commands::ExampleCommand>(
        trussService,
        analysisService,
        presenter,
        args.verbose
    );
    commandPtrs.push_back(exampleCmd.get());
    commands["example"] = std::move(exampleCmd);
    
    // HelpCommand
    auto helpCmd = std::make_unique<truss::cli::commands::HelpCommand>(
        presenter,
        commandPtrs
    );
    commandPtrs.push_back(helpCmd.get());
    commands["help"] = std::move(helpCmd);
    
    // Route and execute
    return executeCommand(args, commands);
}
