/**
 * @file validate_command.cpp
 * @brief CLI command for truss structure validation.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "validate_command.hpp"

#include <filesystem>

namespace truss::cli::commands {

ValidateCommand::ValidateCommand(truss::interface::ITrussAnalysisFacade& facade,
                                 truss::cli::presenters::ConsolePresenter& presenter,
                                 const std::string& inputFile,
                                 bool verbose)
    : m_facade(facade), m_presenter(presenter), m_inputFile(inputFile), m_verbose(verbose) {}

int ValidateCommand::execute() {
    m_presenter.displayHeader();
    m_presenter.displayInfo("Validating truss structure from file...\n");

    // Validate input file
    if (!validateInputFile()) {
        m_presenter.displayError("Input file does not exist or is not readable: " + m_inputFile);
        m_presenter.displayInfo("Suggestion: Check the file path and ensure the file exists.\n");
        m_presenter.displayInfo("Example: TrussAnalysisCLI validate --file path/to/truss.json\n");
        return 1;
    }

    if (m_verbose) {
        m_presenter.displayInfo("Input file: " + m_inputFile + "\n");
    }

    // Use facade to validate from file (single call replaces 4+ service calls)
    auto validateResult = m_facade.validateFromFile(m_inputFile);

    if (!validateResult.isValid()) {
        m_presenter.displayError("Validation failed!");

        // Display validation issues
        const auto& issues = validateResult.getIssues();
        if (!issues.empty()) {
            m_presenter.displayInfo("\nValidation issues found:");
            for (const auto& issue : issues) {
                std::string severity;
                switch (issue.severity) {
                    case truss::core::validation::ValidationSeverity::Fatal:
                        severity = "[FATAL]";
                        break;
                    case truss::core::validation::ValidationSeverity::Error:
                        severity = "[ERROR]";
                        break;
                    case truss::core::validation::ValidationSeverity::Warning:
                        severity = "[WARNING]";
                        break;
                    case truss::core::validation::ValidationSeverity::Info:
                        severity = "[INFO]";
                        break;
                }
                m_presenter.displayInfo("  " + severity + " " + issue.message);
            }
        }

        // Provide suggestions based on issues
        bool hasSupport = false, hasMember = false, hasLoad = false;
        for (const auto& issue : issues) {
            if (issue.category.find("support") != std::string::npos)
                hasSupport = true;
            if (issue.category.find("member") != std::string::npos)
                hasMember = true;
            if (issue.category.find("load") != std::string::npos)
                hasLoad = true;
        }

        if (hasSupport) {
            m_presenter.displayInfo("\nSuggestions for support issues:");
            m_presenter.displayInfo("  - Ensure at least one node has a pinned support");
            m_presenter.displayInfo(
                "  - Check that supports are adequate to prevent rigid body motion");
            m_presenter.displayInfo("  - Verify support types are correctly specified\n");
        }
        if (hasMember) {
            m_presenter.displayInfo("\nSuggestions for member issues:");
            m_presenter.displayInfo("  - Verify all members connect valid nodes");
            m_presenter.displayInfo("  - Check for zero-length or degenerate members");
            m_presenter.displayInfo("  - Ensure member properties are positive\n");
        }
        if (hasLoad) {
            m_presenter.displayInfo("\nSuggestions for loading issues:");
            m_presenter.displayInfo("  - Verify loads are applied to existing nodes");
            m_presenter.displayInfo("  - Check load magnitudes are reasonable");
            m_presenter.displayInfo("  - Ensure at least one load is applied\n");
        }
        if (!hasSupport && !hasMember && !hasLoad) {
            m_presenter.displayInfo("\nGeneral suggestions:");
            m_presenter.displayInfo("  - Review truss geometry and connectivity");
            m_presenter.displayInfo("  - Verify all required properties are specified");
            m_presenter.displayInfo("  - Check for numerical issues (very small/large values)\n");
        }

        return 1;
    }

    // Validation passed
    m_presenter.displaySuccess("\n✓ All validation checks passed!");

    if (m_verbose) {
        m_presenter.displayInfo("\nValidation categories checked:");
        m_presenter.displayInfo("  ✓ Structural validity (nodes, members, connectivity)");
        m_presenter.displayInfo("  ✓ Support conditions (adequate constraints)");
        m_presenter.displayInfo("  ✓ Loading conditions");
        m_presenter.displayInfo("  ✓ Material properties");
        m_presenter.displayInfo("  ✓ Section properties");
        m_presenter.displayInfo("  ✓ Geometry (member lengths, angles)");
        m_presenter.displayInfo("  ✓ Stability");
        m_presenter.displayInfo("  ✓ Numerical conditioning\n");
    }

    m_presenter.displayInfo("\nThe truss structure is valid and ready for analysis.");
    m_presenter.displayInfo("Use 'TrussAnalysisCLI analyze --file " + m_inputFile +
                            "' to run analysis.\n");

    // Automatic cleanup via facade (no manual clearTruss needed)
    return 0;
}

std::string ValidateCommand::getName() const {
    return "validate";
}

std::string ValidateCommand::getDescription() const {
    return "Validate truss structure from file without performing analysis";
}

bool ValidateCommand::validateInputFile() const {
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

}  // namespace truss::cli::commands
