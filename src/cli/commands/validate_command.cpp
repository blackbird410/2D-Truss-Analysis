/**
 * @file validate_command.cpp
 * @brief Implements the validate command.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 */

#include "validate_command.hpp"

#include <filesystem>

namespace truss::cli::commands {

ValidateCommand::ValidateCommand(truss::application::TrussApplicationService& trussService,
                                 truss::cli::presenters::ConsolePresenter& presenter,
                                 const std::string& inputFile,
                                 bool verbose)
    : m_trussService(trussService), m_presenter(presenter), m_inputFile(inputFile),
      m_verbose(verbose) {}

int ValidateCommand::execute() {
    using namespace truss::application;

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

    // Load truss from file
    auto loadResult = m_trussService.loadTruss(m_inputFile);
    if (!loadResult) {
        m_presenter.displayError("Failed to load truss: " + loadResult.errorMessage);
        m_presenter.displayInfo(
            "Suggestion: Verify the file format is valid (JSON, XML, CSV, etc.).\n");
        m_presenter.displayInfo("Supported formats: JSON, XML, CSV, TSV, TXT, YAML\n");
        return 1;
    }
    TrussHandle trussHandle = loadResult.value;

    if (m_verbose) {
        m_presenter.displaySuccess("Truss loaded successfully.");
    }

    // Display truss statistics
    const auto& trussView = m_trussService.getTrussView(trussHandle);
    m_presenter.displayTrussStatistics(trussView);

    // Validate truss structure
    m_presenter.displayInfo("\nRunning validation checks...\n");
    auto validateResult = m_trussService.validateTruss(trussHandle);

    if (!validateResult) {
        m_presenter.displayError("Validation failed: " + validateResult.errorMessage);

        // Provide detailed suggestions based on error message
        std::string errorMsg = validateResult.errorMessage;
        if (errorMsg.find("support") != std::string::npos) {
            m_presenter.displayInfo("\nSuggestions for support issues:");
            m_presenter.displayInfo("  - Ensure at least one node has a pinned support");
            m_presenter.displayInfo(
                "  - Check that supports are adequate to prevent rigid body motion");
            m_presenter.displayInfo("  - Verify support types are correctly specified\n");
        } else if (errorMsg.find("member") != std::string::npos) {
            m_presenter.displayInfo("\nSuggestions for member issues:");
            m_presenter.displayInfo("  - Verify all members connect valid nodes");
            m_presenter.displayInfo("  - Check for zero-length or degenerate members");
            m_presenter.displayInfo("  - Ensure member properties are positive\n");
        } else if (errorMsg.find("load") != std::string::npos) {
            m_presenter.displayInfo("\nSuggestions for loading issues:");
            m_presenter.displayInfo("  - Verify loads are applied to existing nodes");
            m_presenter.displayInfo("  - Check load magnitudes are reasonable");
            m_presenter.displayInfo("  - Ensure at least one load is applied\n");
        } else if (errorMsg.find("material") != std::string::npos) {
            m_presenter.displayInfo("\nSuggestions for material issues:");
            m_presenter.displayInfo("  - Check Young's modulus is positive");
            m_presenter.displayInfo("  - Verify density is positive");
            m_presenter.displayInfo("  - Ensure yield strength is positive\n");
        } else if (errorMsg.find("section") != std::string::npos) {
            m_presenter.displayInfo("\nSuggestions for section issues:");
            m_presenter.displayInfo("  - Verify cross-sectional area is positive");
            m_presenter.displayInfo("  - Check section properties are consistent\n");
        } else if (errorMsg.find("geometry") != std::string::npos) {
            m_presenter.displayInfo("\nSuggestions for geometry issues:");
            m_presenter.displayInfo("  - Check for coincident nodes");
            m_presenter.displayInfo("  - Verify node coordinates are valid");
            m_presenter.displayInfo("  - Ensure geometry is planar (2D)\n");
        } else {
            m_presenter.displayInfo("\nGeneral suggestions:");
            m_presenter.displayInfo("  - Review truss geometry and connectivity");
            m_presenter.displayInfo("  - Verify all required properties are specified");
            m_presenter.displayInfo("  - Check for numerical issues (very small/large values)\n");
        }

        m_trussService.clearTruss(trussHandle);
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

    // Cleanup
    m_trussService.clearTruss(trussHandle);

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
