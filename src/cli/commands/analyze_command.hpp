/**
 * @file analyze_command.hpp
 * @brief Command for file-based truss analysis.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This command implements file-based truss analysis workflow:
 * 1. Load truss from file (JSON/XML/CSV/etc.)
 * 2. Validate structure
 * 3. Run structural analysis
 * 4. Display results
 * 5. Optionally export results to file
 *
 * Architecture: CLI Layer (Command Pattern)
 * Dependencies: Interface Layer (TrussAnalysisFacade)
 */

#pragma once

#include "../../interface/truss_analysis_facade.hpp"
#include "../presenters/console_presenter.hpp"
#include "icommand.hpp"

#include <optional>
#include <string>

namespace truss::cli::commands {

/**
 * @brief Command for analyzing truss structures from files
 *
 * Usage:
 *   TrussAnalysisCLI analyze --file <path> [--output <path>] [--format <format>]
 *
 * Options:
 *   --file, -f     Input file path (required)
 *   --output, -o   Output file path (optional, for results export)
 *   --format       Export format (JSON|XML|CSV|TSV|TXT|YAML|MARKDOWN)
 *   --verbose, -v  Enable verbose output
 *
 * Example:
 *   TrussAnalysisCLI analyze -f bridge.json -o results.csv --format CSV
 */
class AnalyzeCommand : public ICommand {
private:
    truss::interface::TrussAnalysisFacade& m_facade;
    truss::cli::presenters::ConsolePresenter& m_presenter;

    std::string m_inputFile;
    std::optional<std::string> m_outputFile;
    std::optional<std::string> m_exportFormat;
    bool m_verbose;

public:
    /**
     * @brief Construct AnalyzeCommand with dependencies
     *
     * @param facade Interface facade for all analysis operations
     * @param presenter Console presenter for output formatting
     * @param inputFile Path to input truss file
     * @param outputFile Optional output file path for results export
     * @param exportFormat Optional export format (defaults to JSON)
     * @param verbose Enable verbose output
     */
    AnalyzeCommand(truss::interface::TrussAnalysisFacade& facade,
                   truss::cli::presenters::ConsolePresenter& presenter,
                   const std::string& inputFile,
                   const std::optional<std::string>& outputFile = std::nullopt,
                   const std::optional<std::string>& exportFormat = std::nullopt,
                   bool verbose = false);

    /**
     * @brief Execute the analyze command
     *
     * @return Exit code (0 = success, 1+ = error)
     */
    int execute() override;

    /**
     * @brief Get command name
     *
     * @return "analyze"
     */
    std::string getName() const override;

    /**
     * @brief Get command description
     *
     * @return Human-readable description
     */
    std::string getDescription() const override;

private:
    /**
     * @brief Validate input file exists and is readable
     *
     * @return true if valid, false otherwise
     */
    bool validateInputFile() const;

    /**
     * @brief Determine export format from string
     *
     * @param formatStr Format string (case-insensitive)
     * @return Export format enum value, or nullopt if invalid
     */
    static std::optional<truss::ExportFormat> parseExportFormat(const std::string& formatStr);

    /**
     * @brief Get default export format based on file extension
     *
     * @param filepath Output file path
     * @return Export format enum value
     */
    static truss::ExportFormat getDefaultExportFormat(const std::string& filepath);
};

}  // namespace truss::cli::commands
