/**
 * @file main_app.cpp
 * @brief CLI entry point for 2D Truss Analysis.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * Implements Command Pattern with dependency injection.
 * Wires Application services to CLI commands and dispatches execution.
 *
 * Phase 5A.2+5A.4: Updated to wire IApplicationOutput through DI chain.
 */

// Infrastructure Layer includes
#include "infrastructure/adapters/console_output_adapter.hpp"
#include "infrastructure/logging/logger_factory.hpp"

// Application Layer includes
#include "application/analysis_application_service.hpp"
#include "application/truss_application_service.hpp"

// CLI Layer includes
#include "cli/argument_parser.hpp"
#include "cli/commands/analyze_command.hpp"
#include "cli/commands/example_command.hpp"
#include "cli/commands/export_command.hpp"
#include "cli/commands/help_command.hpp"
#include "cli/commands/icommand.hpp"
#include "cli/commands/validate_command.hpp"

#include <map>
#include <memory>
#include <vector>

/**
 * @brief Execute CLI command
 *
 * @param args Parsed command line arguments
 * @param commands Map of registered commands
 * @param presenter Presenter for error messages
 * @return Exit code (0 = success)
 */
int executeCommand(const truss::cli::ParsedArgs& args,
                   std::map<std::string, std::unique_ptr<truss::cli::commands::ICommand>>& commands,
                   const truss::cli::presenters::ConsolePresenter& presenter) {
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
    truss::cli::ParsedArgs args = truss::cli::ArgumentParser::parse(argc, argv);

    // Create command registry
    std::map<std::string, std::unique_ptr<truss::cli::commands::ICommand>> commands;
    std::vector<truss::cli::commands::ICommand*> commandPtrs;

    // Register commands with dependency injection

    // ExampleCommand (always available)
    auto exampleCmd = std::make_unique<truss::cli::commands::ExampleCommand>(
        trussService,
        analysisService,
        presenter,
        args.verbose  // Use parsed verbose flag
    );
    commandPtrs.push_back(exampleCmd.get());
    commands["example"] = std::move(exampleCmd);

    // AnalyzeCommand (create placeholder for help, actual implementation conditionally)
    auto inputFileOpt = truss::cli::ArgumentParser::getOption(args, "file", "f");

    if (inputFileOpt.has_value() && args.commandName == "analyze") {
        std::string inputFile = inputFileOpt.value();

        auto outputFileOpt = truss::cli::ArgumentParser::getOption(args, "output", "o");
        auto formatOpt = truss::cli::ArgumentParser::getOption(args, "format", "f");

        auto analyzeCmd = std::make_unique<truss::cli::commands::AnalyzeCommand>(trussService,
                                                                                 analysisService,
                                                                                 presenter,
                                                                                 inputFile,
                                                                                 outputFileOpt,
                                                                                 formatOpt,
                                                                                 args.verbose);
        commandPtrs.push_back(analyzeCmd.get());
        commands["analyze"] = std::move(analyzeCmd);
    } else {
        // Create placeholder for help display (will show error if executed without proper args)
        auto analyzeCmd = std::make_unique<truss::cli::commands::AnalyzeCommand>(
            trussService,
            analysisService,
            presenter,
            "",  // Empty file will trigger validation error with helpful message
            std::nullopt,
            std::nullopt,
            false  // Default verbose for help display
        );
        commandPtrs.push_back(analyzeCmd.get());
        commands["analyze"] = std::move(analyzeCmd);
    }

    // ValidateCommand (create placeholder for help, actual implementation conditionally)
    if (inputFileOpt.has_value() && args.commandName == "validate") {
        std::string inputFile = inputFileOpt.value();

        auto validateCmd = std::make_unique<truss::cli::commands::ValidateCommand>(
            trussService, presenter, inputFile, args.verbose);
        commandPtrs.push_back(validateCmd.get());
        commands["validate"] = std::move(validateCmd);
    } else {
        // Create placeholder for help display
        auto validateCmd = std::make_unique<truss::cli::commands::ValidateCommand>(
            trussService,
            presenter,
            "",
            args.verbose  // Use parsed verbose flag
        );
        commandPtrs.push_back(validateCmd.get());
        commands["validate"] = std::move(validateCmd);
    }

    // ExportCommand (create placeholder for help, actual implementation conditionally)
    auto trussFileOpt = truss::cli::ArgumentParser::getOption(args, "truss", "t");
    auto resultsFileOpt = truss::cli::ArgumentParser::getOption(args, "results", "r");
    auto exportOutputOpt = truss::cli::ArgumentParser::getOption(args, "output", "o");

    if (trussFileOpt.has_value() && resultsFileOpt.has_value() && exportOutputOpt.has_value() &&
        args.commandName == "export") {
        std::string trussFile = trussFileOpt.value();
        std::string resultsFile = resultsFileOpt.value();
        std::string exportOutput = exportOutputOpt.value();

        auto exportFormatOpt = truss::cli::ArgumentParser::getOption(args, "format", "f");

        auto exportCmd = std::make_unique<truss::cli::commands::ExportCommand>(trussService,
                                                                               analysisService,
                                                                               presenter,
                                                                               trussFile,
                                                                               resultsFile,
                                                                               exportOutput,
                                                                               exportFormatOpt,
                                                                               args.verbose);
        commandPtrs.push_back(exportCmd.get());
        commands["export"] = std::move(exportCmd);
    } else {
        // Create placeholder for help display
        auto exportCmd = std::make_unique<truss::cli::commands::ExportCommand>(
            trussService,
            analysisService,
            presenter,
            "",
            "",
            "",
            std::nullopt,
            args.verbose  // Use parsed verbose flag
        );
        commandPtrs.push_back(exportCmd.get());
        commands["export"] = std::move(exportCmd);
    }

    // HelpCommand (always available)
    auto helpCmd = std::make_unique<truss::cli::commands::HelpCommand>(presenter, commandPtrs);
    commandPtrs.push_back(helpCmd.get());
    commands["help"] = std::move(helpCmd);

    // Route and execute
    return executeCommand(args, commands, presenter);
}
