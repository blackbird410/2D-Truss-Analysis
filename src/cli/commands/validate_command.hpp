/**
 * @file validate_command.hpp
 * @brief Command for validating truss structures without analysis.
 * @version 3.0.0
 * @date 2026-02-24
 * @author Neil Taison Rigaud
 *
 * This command validates truss structures loaded from files without
 * performing structural analysis. Useful for quick structure checks.
 *
 * Architecture: CLI Layer (Command Pattern)
 * Dependencies: Interface Layer (ITrussAnalysisFacade)
 */

#pragma once

#include "../../interface/itruss_analysis_facade.hpp"
#include "../presenters/console_presenter.hpp"
#include "icommand.hpp"

#include <string>

namespace truss::cli::commands {

/**
 * @brief Command for validating truss structures
 *
 * Usage:
 *   TrussAnalysisCLI validate --file <path> [--verbose]
 *
 * Options:
 *   --file, -f     Input file path (required)
 *   --verbose, -v  Enable verbose output with detailed validation results
 *
 * Validation Categories:
 * - Structural validity (nodes, members, connectivity)
 * - Support conditions (adequate constraints)
 * - Loading conditions
 * - Material properties
 * - Section properties
 * - Geometry (member lengths, angles)
 * - Stability
 * - Numerical conditioning
 *
 * Example:
 *   TrussAnalysisCLI validate -f bridge.json -v
 */
class ValidateCommand : public ICommand {
private:
    truss::interface::ITrussAnalysisFacade& m_facade;
    truss::cli::presenters::ConsolePresenter& m_presenter;

    std::string m_inputFile;
    bool m_verbose;

public:
    /**
     * @brief Construct ValidateCommand with dependencies
     *
     * @param facade Interface facade for analysis operations
     * @param presenter Console presenter for output formatting
     * @param inputFile Path to input truss file
     * @param verbose Enable verbose output
     */
    ValidateCommand(truss::interface::ITrussAnalysisFacade& facade,
                    truss::cli::presenters::ConsolePresenter& presenter,
                    const std::string& inputFile,
                    bool verbose = false);

    /**
     * @brief Execute the validate command
     *
     * @return Exit code (0 = valid, 1+ = invalid or error)
     */
    int execute() override;

    /**
     * @brief Get command name
     *
     * @return "validate"
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
};

}  // namespace truss::cli::commands
