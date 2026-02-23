/**
 * @file ExportCommand.hpp
 * @brief Command for exporting analysis results to various formats
 * @author Civil Engineering Software Solutions
 * @version 3.0.0
 *
 * This command exports previously generated analysis results to different
 * file formats. Requires both input truss file and results file.
 *
 * Architecture: CLI Layer (Command Pattern)
 * Dependencies: Application services (TrussApplicationService, AnalysisApplicationService)
 */

#pragma once

#include "../../application/analysis_application_service.hpp"
#include "../../application/truss_application_service.hpp"
#include "../presenters/console_presenter.hpp"
#include "icommand.hpp"

#include <optional>
#include <string>

namespace truss::cli::commands {

/**
 * @brief Command for exporting analysis results
 *
 * Usage:
 *   TrussAnalysisCLI export --truss <path> --results <path> --output <path> [--format <format>]
 *
 * Options:
 *   --truss, -t    Input truss file path (required)
 *   --results, -r  Input results file path (required)
 *   --output, -o   Output file path (required)
 *   --format, -f   Export format (JSON|XML|CSV|TSV|TXT|YAML|MARKDOWN)
 *   --verbose, -v  Enable verbose output
 *
 * Supported Export Formats:
 * - JSON: Structured data with full details
 * - XML: Standard XML format
 * - CSV: Comma-separated values (tabular data)
 * - TSV: Tab-separated values
 * - TXT: Human-readable text format
 * - YAML: YAML format
 * - MARKDOWN: Formatted markdown tables
 *
 * Example:
 *   TrussAnalysisCLI export -t bridge.json -r results.json -o report.md --format MARKDOWN
 */
class ExportCommand : public ICommand {
private:
    truss::application::TrussApplicationService& m_trussService;
    truss::application::AnalysisApplicationService& m_analysisService;
    truss::cli::presenters::ConsolePresenter& m_presenter;

    std::string m_trussFile;
    std::string m_resultsFile;
    std::string m_outputFile;
    std::optional<std::string> m_exportFormat;
    bool m_verbose;

public:
    /**
     * @brief Construct ExportCommand with dependencies
     *
     * @param trussService Application service for truss operations
     * @param analysisService Application service for analysis operations
     * @param presenter Console presenter for output formatting
     * @param trussFile Path to input truss file
     * @param resultsFile Path to input results file
     * @param outputFile Path to output export file
     * @param exportFormat Optional export format (defaults to format from file extension)
     * @param verbose Enable verbose output
     */
    ExportCommand(truss::application::TrussApplicationService& trussService,
                  truss::application::AnalysisApplicationService& analysisService,
                  truss::cli::presenters::ConsolePresenter& presenter,
                  const std::string& trussFile,
                  const std::string& resultsFile,
                  const std::string& outputFile,
                  const std::optional<std::string>& exportFormat = std::nullopt,
                  bool verbose = false);

    /**
     * @brief Execute the export command
     *
     * @return Exit code (0 = success, 1+ = error)
     */
    int execute() override;

    /**
     * @brief Get command name
     *
     * @return "export"
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
     * @param filepath File path to validate
     * @return true if valid, false otherwise
     */
    static bool validateInputFile(const std::string& filepath);

    /**
     * @brief Determine export format from string
     *
     * @param formatStr Format string (case-insensitive)
     * @return Export format enum value, or nullopt if invalid
     */
    static std::optional<truss::infrastructure::export_::ExportFormat>
    parseExportFormat(const std::string& formatStr);

    /**
     * @brief Get default export format based on file extension
     *
     * @param filepath Output file path
     * @return Export format enum value
     */
    static truss::infrastructure::export_::ExportFormat
    getDefaultExportFormat(const std::string& filepath);
};

}  // namespace truss::cli::commands
