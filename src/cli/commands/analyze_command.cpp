/**
 * @file analyze_command.cpp
 * @brief CLI command for truss analysis execution.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "analyze_command.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace truss::cli::commands {

AnalyzeCommand::AnalyzeCommand(truss::interface::TrussAnalysisFacade& facade,
                               truss::cli::presenters::ConsolePresenter& presenter,
                               const std::string& inputFile,
                               const std::optional<std::string>& outputFile,
                               const std::optional<std::string>& exportFormat,
                               bool verbose)
    : m_facade(facade), m_presenter(presenter), m_inputFile(inputFile),
      m_outputFile(outputFile), m_exportFormat(exportFormat), m_verbose(verbose) {}

int AnalyzeCommand::execute() {
    using namespace truss::infrastructure::export_;

    m_presenter.displayHeader();
    m_presenter.displayInfo("Analyzing truss structure from file...\n");

    // Validate input file
    if (!validateInputFile()) {
        m_presenter.displayError("Input file does not exist or is not readable: " + m_inputFile);
        m_presenter.displayInfo("Suggestion: Check the file path and ensure the file exists.\n");
        return 1;
    }

    if (m_verbose) {
        m_presenter.displayInfo("Input file: " + m_inputFile + "\n");
    }

    // ===== SINGLE FACADE CALL REPLACES 5+ SERVICE CALLS =====
    // Old approach: loadTruss(), getTrussView(), getTrussMutable(),
    //               validateTruss(), analyze()
    // New approach: analyzeFromFile() does all of the above
    auto analysisResult = m_facade.analyzeFromFile(m_inputFile);

    if (!analysisResult) {
        m_presenter.displayError("Analysis failed: " + analysisResult.errorMessage);
        m_presenter.displayInfo(
            "Suggestion: Check the file format and ensure the structure is valid.\n");
        return 1;
    }

    if (m_verbose) {
        m_presenter.displaySuccess("Analysis completed successfully.");
    }

    // Display truss statistics
    const auto& trussView = m_facade.getTrussView(analysisResult.trussHandle);
    m_presenter.displayTrussStatistics(trussView);

    // Display results
    const auto& resultsView = m_facade.getResultsView(analysisResult.resultsHandle);
    m_presenter.displayAnalysisResults(resultsView);

    // Export results if requested
    if (m_outputFile.has_value()) {
        ExportFormat format;

        // Determine format
        if (m_exportFormat.has_value()) {
            auto parsedFormat = parseExportFormat(m_exportFormat.value());
            if (!parsedFormat.has_value()) {
                m_presenter.displayError("Invalid export format: " + m_exportFormat.value());
                m_presenter.displayInfo("Valid formats: JSON, XML, CSV, TSV, TXT, LaTeX, HTML\n");
                return 1;
            }
            format = parsedFormat.value();
        } else {
            format = getDefaultExportFormat(m_outputFile.value());
        }

        if (m_verbose) {
            m_presenter.displayInfo("\nExporting results to: " + m_outputFile.value() + "\n");
        }

        // Use facade export method
        if (!m_facade.exportResults(analysisResult.resultsHandle, format, m_outputFile.value())) {
            m_presenter.displayError("Export failed: Check output directory is writable.");
            m_presenter.displayInfo(
                "Suggestion: Verify that the output directory exists and you have write permissions.\n");
            return 1;
        }

        m_presenter.displaySuccess("Results exported to " + m_outputFile.value());
    }

    // Automatic cleanup via RAII (resources held by facade)
    // No manual clearResults() or clearTruss() needed

    m_presenter.displaySuccess("\nAnalysis complete!");
    return 0;
}

std::string AnalyzeCommand::getName() const {
    return "analyze";
}

std::string AnalyzeCommand::getDescription() const {
    return "Analyze truss structure from file (supports JSON, XML, CSV, etc.)";
}

bool AnalyzeCommand::validateInputFile() const {
    namespace fs = std::filesystem;

    try {
        if (!fs::exists(m_inputFile)) {
            return false;
        }

        if (!fs::is_regular_file(m_inputFile)) {
            return false;
        }

        // Check read permissions
        auto perms = fs::status(m_inputFile).permissions();
        return (perms & fs::perms::owner_read) != fs::perms::none;

    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::optional<truss::infrastructure::export_::ExportFormat>
AnalyzeCommand::parseExportFormat(const std::string& formatStr) {
    using namespace truss::infrastructure::export_;

    // Convert to uppercase for case-insensitive comparison
    std::string upper = formatStr;
    std::transform(
        upper.begin(), upper.end(), upper.begin(), [](unsigned char c) { return std::toupper(c); });

    if (upper == "JSON")
        return ExportFormat::JSON;
    if (upper == "XML")
        return ExportFormat::XML;
    if (upper == "CSV")
        return ExportFormat::CSV;
    if (upper == "TSV")
        return ExportFormat::TSV;
    if (upper == "TXT")
        return ExportFormat::TXT;
    if (upper == "LATEX" || upper == "TEX")
        return ExportFormat::LaTeX;
    if (upper == "HTML")
        return ExportFormat::HTML;

    return std::nullopt;
}

truss::infrastructure::export_::ExportFormat
AnalyzeCommand::getDefaultExportFormat(const std::string& filepath) {
    using namespace truss::infrastructure::export_;

    namespace fs = std::filesystem;
    std::string ext = fs::path(filepath).extension().string();

    // Convert to lowercase
    std::transform(
        ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

    if (ext == ".json")
        return ExportFormat::JSON;
    if (ext == ".xml")
        return ExportFormat::XML;
    if (ext == ".csv")
        return ExportFormat::CSV;
    if (ext == ".tsv")
        return ExportFormat::TSV;
    if (ext == ".txt")
        return ExportFormat::TXT;
    if (ext == ".tex")
        return ExportFormat::LaTeX;
    if (ext == ".html")
        return ExportFormat::HTML;

    // Default to JSON
    return ExportFormat::JSON;
}

}  // namespace truss::cli::commands
