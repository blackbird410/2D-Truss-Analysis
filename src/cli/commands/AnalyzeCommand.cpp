/**
 * @file AnalyzeCommand.cpp
 * @brief Implementation of analyze command
 */

#include "AnalyzeCommand.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace truss::cli::commands {

AnalyzeCommand::AnalyzeCommand(truss::application::TrussApplicationService& trussService,
                               truss::application::AnalysisApplicationService& analysisService,
                               truss::cli::presenters::ConsolePresenter& presenter,
                               const std::string& inputFile,
                               const std::optional<std::string>& outputFile,
                               const std::optional<std::string>& exportFormat,
                               bool verbose)
    : m_trussService(trussService), m_analysisService(analysisService), m_presenter(presenter),
      m_inputFile(inputFile), m_outputFile(outputFile), m_exportFormat(exportFormat),
      m_verbose(verbose) {}

int AnalyzeCommand::execute() {
    using namespace truss::application;
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

    // Load truss from file
    auto loadResult = m_trussService.loadTruss(m_inputFile);
    if (!loadResult) {
        m_presenter.displayError("Failed to load truss: " + loadResult.errorMessage);
        m_presenter.displayInfo(
            "Suggestion: Verify the file format is valid (JSON, XML, CSV, etc.).\n");
        return 1;
    }
    TrussHandle trussHandle = loadResult.value;

    if (m_verbose) {
        m_presenter.displaySuccess("Truss loaded successfully.");
    }

    // Display truss statistics
    const auto& trussView = m_trussService.getTrussView(trussHandle);
    m_presenter.displayTrussStatistics(trussView);

    // Validate truss
    auto validateResult = m_trussService.validateTruss(trussHandle);
    if (!validateResult) {
        m_presenter.displayError("Truss validation failed: " + validateResult.errorMessage);
        m_presenter.displayInfo(
            "Suggestion: Check truss geometry, supports, and loading conditions.\n");
        m_trussService.clearTruss(trussHandle);
        return 1;
    }

    if (m_verbose) {
        m_presenter.displaySuccess("Truss validation passed.");
    }

    // Get mutable truss for analysis
    auto& truss = m_trussService.getTrussMutable(trussHandle);

    // Run analysis
    m_presenter.displayInfo("\nRunning structural analysis...\n");
    auto analysisResult = m_analysisService.analyze(truss);
    if (!analysisResult) {
        m_presenter.displayError("Analysis failed: " + analysisResult.errorMessage);
        m_presenter.displayInfo("Suggestion: Check that the truss is statically determinate and "
                                "properly constrained.\n");
        m_trussService.clearTruss(trussHandle);
        return 1;
    }
    ResultsHandle resultsHandle = analysisResult.value;

    if (m_verbose) {
        m_presenter.displaySuccess("Analysis completed successfully.");
    }

    // Display results
    const auto& resultsView = m_analysisService.getResultsView(resultsHandle);
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
                m_analysisService.clearResults(resultsHandle);
                m_trussService.clearTruss(trussHandle);
                return 1;
            }
            format = parsedFormat.value();
        } else {
            format = getDefaultExportFormat(m_outputFile.value());
        }

        if (m_verbose) {
            m_presenter.displayInfo("\nExporting results to: " + m_outputFile.value() + "\n");
        }

        // Export via Application service
        auto exportResult = m_analysisService.exportResults(
            resultsHandle, format, m_outputFile.value(), truss, {}  // Default export options
        );

        if (!exportResult) {
            m_presenter.displayError("Export failed: " + exportResult.errorMessage);
            m_presenter.displayInfo(
                "Suggestion: Check that the output directory exists and is writable.\n");
            m_analysisService.clearResults(resultsHandle);
            m_trussService.clearTruss(trussHandle);
            return 1;
        }

        m_presenter.displaySuccess("Results exported to " + m_outputFile.value());
    }

    // Cleanup
    m_analysisService.clearResults(resultsHandle);
    m_trussService.clearTruss(trussHandle);

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
AnalyzeCommand::parseExportFormat(const std::string& formatStr) const {
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
AnalyzeCommand::getDefaultExportFormat(const std::string& filepath) const {
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
