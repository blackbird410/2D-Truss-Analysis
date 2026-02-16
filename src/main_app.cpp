/**
 * @file main_app.cpp
 * @brief CLI entry point for 2D Truss Analysis
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 * 
 * Implements Command Pattern with dependency injection.
 * Wires Application services to CLI commands and dispatches execution.
 * 
 * Phase 5A.2+5A.4: Updated to wire IApplicationOutput through DI chain.
 */

// Infrastructure Layer includes
#include "infrastructure/logging/logger_factory.hpp"
#include "infrastructure/adapters/ConsoleOutputAdapter.hpp"

// Application Layer includes
#include "application/TrussApplicationService.hpp"
#include "application/AnalysisApplicationService.hpp"

// CLI Layer includes
#include "cli/ArgumentParser.hpp"
#include "cli/commands/ICommand.hpp"
#include "cli/commands/ExampleCommand.hpp"
#include "cli/commands/HelpCommand.hpp"
#include "cli/presenters/ConsolePresenter.hpp"

#include <memory>
#include <map>
#include <vector>

/**
 * @brief Execute CLI command
 * 
 * @param args Parsed command line arguments
 * @param commands Map of registered commands
 * @param presenter Presenter for error messages
 * @return Exit code (0 = success)
 */
int executeCommand(
    const truss::cli::ParsedArgs& args,
    std::map<std::string, std::unique_ptr<truss::cli::commands::ICommand>>& commands,
    truss::cli::presenters::ConsolePresenter& presenter
) {
    auto it = commands.find(args.commandName);
    if (it == commands.end()) {
        presenter.displayError("Unknown command '" + args.commandName + "'");
        presenter.displayInfo("Run 'TrussAnalysisCLI help' for available commands.");
        return 1;
    }
    
    return it->second->execute();
}

/**
 * @brief CLI entry point
 * 
 * Wires dependencies and dispatches command execution:
 * 1. Creates Infrastructure logger and output adapter
 * 2. Creates Application services and presenters
 * 3. Parses command-line arguments
 * 4. Registers commands with dependency injection
 * 5. Routes to appropriate command and executes
 * 
 * Phase 5A.4: Complete DI chain - Logger → Adapter → Presenter → Commands
 */
int main(int argc, char* argv[]) {
    // Create Infrastructure dependencies (innermost layer)
    auto logger = truss::infrastructure::logging::LoggerFactory::createConsoleLogger(
        truss::infrastructure::logging::LogLevel::Info,
        true  // enable colors
    );
    
    // Create Application output adapter (bridges layers)
    truss::infrastructure::adapters::ConsoleOutputAdapter consoleOutput(*logger);
    
    // Create presenter with output dependency (CLI layer)
    truss::cli::presenters::ConsolePresenter presenter(consoleOutput);
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
    return executeCommand(args, commands, presenter);
}
