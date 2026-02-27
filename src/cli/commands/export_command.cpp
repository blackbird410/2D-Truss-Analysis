/**
 * @file export_command.cpp
 * @brief CLI command for results export to various formats.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "export_command.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace truss::cli::commands {

ExportCommand::ExportCommand(truss::interface::TrussAnalysisFacade& facade,
                             truss::cli::presenters::ConsolePresenter& presenter,
                             const std::string& trussFile,
                             const std::string& resultsFile,
                             const std::string& outputFile,
                             const std::optional<std::string>& exportFormat,
                             bool verbose)
    : m_facade(facade), m_presenter(presenter), m_trussFile(trussFile),
      m_resultsFile(resultsFile), m_outputFile(outputFile),
      m_exportFormat(exportFormat), m_verbose(verbose) {}

int ExportCommand::execute() {
    using namespace truss::infrastructure::export_;

    m_presenter.displayHeader();
    m_presenter.displayInfo("Exporting analysis results...\n");

    // Validate input files
    if (!validateInputFile(m_trussFile)) {
        m_presenter.displayError("Truss file does not exist or is not readable: " + m_trussFile);
        m_presenter.displayInfo("Suggestion: Check the truss file path and ensure it exists.\n");
        return 1;
    }

    if (m_verbose) {
        m_presenter.displayInfo("Truss file: " + m_trussFile + "\n");
        m_presenter.displayInfo("Output file: " + m_outputFile + "\n");
    }

    // Analyze truss from file using facade
    m_presenter.displayInfo("\nAnalyzing truss to generate results for export...\n");
    auto analysisResult = m_facade.analyzeFromFile(m_trussFile);

    if (!analysisResult) {
        m_presenter.displayError("Analysis failed: " + analysisResult.errorMessage);
        m_presenter.displayInfo("Suggestion: Validate the truss structure first.\n");
        m_presenter.displayInfo("Example: TrussAnalysisCLI validate --file " + m_trussFile + "\n");
        return 1;
    }

    if (m_verbose) {
        m_presenter.displaySuccess("Analysis completed successfully.");
    }

    // Determine export format
    ExportFormat format;
    if (m_exportFormat.has_value()) {
        auto parsedFormat = parseExportFormat(m_exportFormat.value());
        if (!parsedFormat.has_value()) {
            m_presenter.displayError("Invalid export format: " + m_exportFormat.value());
            m_presenter.displayInfo("Valid formats: JSON, XML, CSV, TSV, TXT, LaTeX, HTML\n");
            return 1;
        }
        format = parsedFormat.value();
    } else {
        format = getDefaultExportFormat(m_outputFile);
    }

    if (m_verbose) {
        std::string formatName;
        switch (format) {
            case ExportFormat::JSON:
                formatName = "JSON";
                break;
            case ExportFormat::XML:
                formatName = "XML";
                break;
            case ExportFormat::CSV:
                formatName = "CSV";
                break;
            case ExportFormat::TSV:
                formatName = "TSV";
                break;
            case ExportFormat::TXT:
                formatName = "TXT";
                break;
            case ExportFormat::LaTeX:
                formatName = "LaTeX";
                break;
            case ExportFormat::HTML:
                formatName = "HTML";
                break;
        }
        m_presenter.displayInfo("Export format: " + formatName + "\n");
    }

    // Export results using facade
    m_presenter.displayInfo("\nExporting results...\n");
    if (!m_facade.exportResults(analysisResult.resultsHandle, format, m_outputFile)) {
        m_presenter.displayError("Export failed: Check output directory is writable.");
        m_presenter.displayInfo(
            "Suggestion: Verify that the output directory exists and you have write permissions.\n");
        return 1;
    }

    // Automatic cleanup via facade RAII (no manual clearResults or clearTruss needed)

    m_presenter.displaySuccess("\n✓ Results exported successfully to " + m_outputFile);

    // Show file size
    try {
        namespace fs = std::filesystem;
        auto fileSize = fs::file_size(m_outputFile);
        m_presenter.displayInfo("File size: " + std::to_string(fileSize) + " bytes\n");
    } catch (...) {
        // Ignore file size errors
    }

    return 0;
}

std::string ExportCommand::getName() const {
    return "export";
}

std::string ExportCommand::getDescription() const {
    return "Export analysis results to various formats (JSON, XML, CSV, MARKDOWN, etc.)";
}

bool ExportCommand::validateInputFile(const std::string& filepath) {
    namespace fs = std::filesystem;

    try {
        if (!fs::exists(filepath)) {
            return false;
        }

        if (!fs::is_regular_file(filepath)) {
            return false;
        }

        // Check read permissions
        auto perms = fs::status(filepath).permissions();
        return (perms & fs::perms::owner_read) != fs::perms::none;

    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

std::optional<truss::infrastructure::export_::ExportFormat>
ExportCommand::parseExportFormat(const std::string& formatStr) {
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
ExportCommand::getDefaultExportFormat(const std::string& filepath) {
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
